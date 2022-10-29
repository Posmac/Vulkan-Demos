#pragma once

#include "vkDebugMesenger.h"
#include "vkInstance.h"
#include "vkLogicalDevice.h"
#include "vkPhysicalDevice.h"
#include "vkSwapchain.h"
#include "Win32Window.h"

namespace vk
{
	class VkApp
	{
	public:
		VkApp();
		void Destroy();
		void Run(int width, int height);
		bool IsRunning();
		void Update();
	private:
		void QuerryAvailableLayers();
	private:
		int width;
		int height;

		uint32_t availableLayersCount;
		std::vector<VkLayerProperties> availableLayers;

		bool debugModeEnabled;
		CVkDebugMessenger debugMessenger;
		CVkInstance instance;
		CVkPhysicalDevice gpu;
		CVkLogicalDevice device;
		CVkSwapchain swapchain;
		Win32Window window;

	private:
		//TODO: abstract in some structure later
		const std::vector<const char*> usedValidationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		const std::vector<const char*> usedInstanceExtensions =
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
			"VK_EXT_debug_report",
			"VK_EXT_debug_utils"
		};

		const std::vector<const char*> usedDeviceExtensions = {
			"VK_KHR_swapchain",
		};

		//buffers
		void BeginCommandBufferRecord(VkCommandBuffer buffer, VkCommandBufferUsageFlags flags)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = flags;
			beginInfo.pNext = nullptr;
			beginInfo.pInheritanceInfo = nullptr;

			VK_CHECK_RESULT(vkBeginCommandBuffer(buffer, &beginInfo));
		}

		void EndCommandBufferRecord(VkCommandBuffer buffer)
		{
			VK_CHECK_RESULT(vkEndCommandBuffer(buffer));

		}

		VkFence CreateFence(VkDevice const& device, VkFenceCreateFlags flags)
		{
			VkFence fence;

			VkFenceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = flags;

			VK_CHECK_RESULT(vkCreateFence(device, &createInfo, nullptr, &fence));
			return fence;
		}

		struct WaitSemaphoreInfo {
			VkSemaphore           Semaphore;
			VkPipelineStageFlags  WaitingStage;
		};

		void SubmitCommandBuffersToQueue(VkQueue queue,
			std::vector<WaitSemaphoreInfo> waitSemaphoresInfo,
			std::vector<VkCommandBuffer> cmdBufs,
			std::vector<VkSemaphore> signalSemaphores,
			VkFence fence) 
		{
			std::vector<VkSemaphore> waitSemaphoreHandles;
			std::vector<VkPipelineStageFlags> waitSemaphoreStagers;

			for (auto& wait_semaphore_info : waitSemaphoresInfo)
			{
				waitSemaphoreHandles.emplace_back(wait_semaphore_info.Semaphore);
				waitSemaphoreStagers.emplace_back(wait_semaphore_info.WaitingStage);
			}

			VkSubmitInfo submit_info = 
			{
			  VK_STRUCTURE_TYPE_SUBMIT_INFO,
			  nullptr,
			  static_cast<uint32_t>(waitSemaphoresInfo.size()),
			  waitSemaphoreHandles.data(),
			  waitSemaphoreStagers.data(),
			  static_cast<uint32_t>(cmdBufs.size()),
			  cmdBufs.data(),
			  static_cast<uint32_t>(signalSemaphores.size()),
			  signalSemaphores.data()
			};

			VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, fence));
		}

		void WaitForFences(VkDevice device, std::vector<VkFence> const& fences,
			VkBool32 waitForAllFences, uint64_t timeOut) 
		{
			if (fences.size() > 0) 
			{
				VK_CHECK_RESULT(vkWaitForFences(device, static_cast<uint32_t>(fences.size()), 
					fences.data(), waitForAllFences, timeOut));
			}
		}

		//resources and memoryv

		VkImage CreateImage(const VkDevice& device, VkImageType type, VkFormat format, VkExtent3D size, uint32_t mipmapLevels,
			uint32_t layers, VkSampleCountFlagBits samples, VkImageUsageFlags usageFlags)
		{
			VkImageCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.imageType = type;
			info.format = format;
			info.extent = size;
			info.mipLevels = mipmapLevels;
			info.arrayLayers = layers;
			info.samples = samples;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			info.usage = usageFlags;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VkImage image = VK_NULL_HANDLE;
			VK_CHECK_RESULT(vkCreateImage(device, &info, nullptr, &image));
			return image;
		}

		void AllocateImageMemory(VkPhysicalDevice gpu, VkDevice device, VkImage image, VkMemoryPropertyFlagBits flags)
		{
			//for performance store physical device memory props after selecting suitable one
			VkPhysicalDeviceMemoryProperties gpuMemProps{};
			vkGetPhysicalDeviceMemoryProperties(gpu, &gpuMemProps);

			VkMemoryRequirements memReq{};
			vkGetImageMemoryRequirements(device, image, &memReq);

			VkDeviceMemory deviceMem = VK_NULL_HANDLE;

			for (uint32_t type = 0; type < gpuMemProps.memoryTypeCount; type++)
			{
				if ((memReq.memoryTypeBits & (1 << type)) &&
					(gpuMemProps.memoryTypes[type].propertyFlags & flags) == flags)
				{
					VkMemoryAllocateInfo allocInfo{};
					allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
					allocInfo.pNext = nullptr;
					allocInfo.allocationSize = memReq.size;
					allocInfo.memoryTypeIndex = type;

					VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &deviceMem));
				}
			}

			VK_CHECK_RESULT(vkBindImageMemory(device, image, deviceMem, 0));
		}

		struct ImageTransition
		{
			VkImage image;
			VkAccessFlags srcAccessFlags;
			VkAccessFlags dstAccessFlags;
			VkImageLayout srcLayout;
			VkImageLayout dstLayout;
			uint32_t srcQueueFamilyIndex;
			uint32_t dstQueueFamilyIndex;
			VkImageAspectFlags aspect;
		};

		ImageTransition SetupImageTransition(VkImage image, VkAccessFlags srcAccessFlags,
			VkAccessFlags dstAccessFlags, VkImageLayout srcLayout, VkImageLayout dstLayout,
			uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkImageAspectFlags aspect)
		{
			ImageTransition transition{};
			transition.image = image;
			transition.srcAccessFlags = srcAccessFlags;
			transition.dstAccessFlags = dstAccessFlags;
			transition.srcLayout = srcLayout;
			transition.dstLayout = dstLayout;
			transition.srcQueueFamilyIndex = srcQueueFamilyIndex;
			transition.dstQueueFamilyIndex = dstQueueFamilyIndex;
			transition.aspect = aspect;
			return transition;
		}

		VkImageMemoryBarrier SetupImageMemoryBarrier(const ImageTransition& transition)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.image = transition.image;
			barrier.srcAccessMask = transition.srcAccessFlags;
			barrier.dstAccessMask = transition.dstAccessFlags;
			barrier.oldLayout = transition.srcLayout;
			barrier.newLayout = transition.dstLayout;
			barrier.srcQueueFamilyIndex = transition.srcQueueFamilyIndex;
			barrier.dstQueueFamilyIndex = transition.dstQueueFamilyIndex;
			barrier.subresourceRange.aspectMask = transition.aspect;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

			return barrier;
		}

		void RecordImageTransition(VkCommandBuffer cmdBuf, VkImageMemoryBarrier barrier,
			VkPipelineStageFlags srcPipelineFlags, VkPipelineStageFlags dstPipelineFlags)
		{
			//command buffer must be in record state before transition

			VkImageMemoryBarrier barriers[1] = { barrier };

			vkCmdPipelineBarrier(cmdBuf, srcPipelineFlags, dstPipelineFlags, 0, 0, nullptr, 0, nullptr,
				static_cast<uint32_t>(1), barriers);

			//command buffer must end record state after transition
		}

		VkImageView CreateImageView(VkDevice device, VkImage image, VkImageViewType viewType, VkFormat format,
			VkImageAspectFlags aspectMask)
		{
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.image = image;
			info.viewType = viewType;
			info.format = format;
			info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.subresourceRange.aspectMask = aspectMask;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
			info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

			VkImageView view = VK_NULL_HANDLE;
			VK_CHECK_RESULT(vkCreateImageView(device, &info, nullptr, &view));
			return view;
		}

		VkBuffer CreateBuffer(const VkDevice& device, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkSharingMode sharingMode)
		{
			VkDeviceSize deviceSize{};
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.pNext = nullptr;
			bufferInfo.flags = 0;
			bufferInfo.usage = usageFlags;
			bufferInfo.sharingMode = sharingMode;
			bufferInfo.size = size;
			bufferInfo.queueFamilyIndexCount = 0;
			bufferInfo.pQueueFamilyIndices = nullptr;

			VkBuffer buffer{};
			VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));
			return buffer;
		}

		VkDeviceMemory AllocateBufferMemory(VkPhysicalDevice gpu, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags flags)
		{
			//for performance store physical device memory props after selecting suitable one
			VkPhysicalDeviceMemoryProperties gpuMemProps{};
			vkGetPhysicalDeviceMemoryProperties(gpu, &gpuMemProps);

			VkMemoryRequirements memReq{};
			vkGetBufferMemoryRequirements(device, buffer, &memReq);

			VkDeviceMemory deviceMem = VK_NULL_HANDLE;

			for (uint32_t type = 0; type < gpuMemProps.memoryTypeCount; type++)
			{
				if ((memReq.memoryTypeBits & (1 << type)) &&
					(gpuMemProps.memoryTypes[type].propertyFlags & flags) == flags)
				{
					VkMemoryAllocateInfo allocInfo{};
					allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
					allocInfo.pNext = nullptr;
					allocInfo.allocationSize = memReq.size;
					allocInfo.memoryTypeIndex = type;

					VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &deviceMem));
				}
			}

			VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, deviceMem, 0));

			return deviceMem;
		}

		struct BufferTransition
		{
			VkBuffer buffer;
			VkAccessFlags srcFlags;
			VkAccessFlags dstFlags;
			uint32_t srcQueueFamilyIndex;
			uint32_t dstQueueFamilyIndex;
		};

		void SetupBufferTransition(BufferTransition& trans, VkBuffer buffer, VkAccessFlags srcFlag,
			VkAccessFlags dstFlag, uint32_t srcQueueFamiliyIndex, uint32_t dstQueueFamilyIndex)
		{
			trans.buffer = buffer;
			trans.srcFlags = srcFlag;
			trans.dstFlags = dstFlag;
			trans.srcQueueFamilyIndex = srcQueueFamiliyIndex;
			trans.dstQueueFamilyIndex = dstQueueFamilyIndex;
		}

		VkBufferMemoryBarrier SetupBufferMemoryBarrier(BufferTransition transition, VkDeviceSize size, VkDeviceSize offset)
		{
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.size = size;
			barrier.offset = offset;

			barrier.buffer = transition.buffer;
			barrier.srcAccessMask = transition.srcFlags;
			barrier.dstAccessMask = transition.dstFlags;
			barrier.srcQueueFamilyIndex = transition.srcQueueFamilyIndex;
			barrier.dstQueueFamilyIndex = transition.dstQueueFamilyIndex;

			return barrier;
		}

		void RecordBufferTransition(VkCommandBuffer cmdBuffer, VkBufferMemoryBarrier bufferBarrier,
			VkPipelineStageFlags srcPipelineFlags, VkPipelineStageFlags dstPipelineFlags)
		{
			//command buffer must be in record state

			VkBufferMemoryBarrier barriers[1] = { bufferBarrier };

			vkCmdPipelineBarrier(cmdBuffer, srcPipelineFlags, dstPipelineFlags, 0, 0, nullptr,
				static_cast<uint32_t>(1), barriers, 0, nullptr);

			//at the end finish command buffer record
		}

		VkBufferView CreateBufferView(VkDevice device, VkBuffer buffer, VkFormat format,
			VkDeviceSize sizze, VkDeviceSize offset)
		{
			VkBufferViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.buffer = buffer;
			info.format = format;
			info.range = sizze;
			info.offset = offset;

			VkBufferView view;
			VK_CHECK_RESULT(vkCreateBufferView(device, &info, nullptr, &view));
			return view;
		}

		struct Image
		{
			VkImage image;
			VkImageView view;
		};

		Image Create2DImageRGBA32(VkDevice device, VkPhysicalDevice gpu, VkExtent2D size, uint32_t mipmapLevels,
			uint32_t layersCount, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageAspectFlags aspectMask)
		{
			Image img{};
			VkExtent3D size2D = { size.width, size.height, 1 };
			img.image = CreateImage(device, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, size2D,
				mipmapLevels, layersCount, samples, usage);
			AllocateImageMemory(gpu, device, img.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			img.view = CreateImageView(device, img.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, aspectMask);
			return img;
		}

		void MapUpdateAndUnmapHostVisibleMemory(VkDevice device, VkDeviceMemory memoryObject,
			VkDeviceSize offset, VkDeviceSize dataSize, void* dataToCopy)
		{
			void* mappedMemory;
			VK_CHECK_RESULT(vkMapMemory(device, memoryObject, offset, dataSize, 0,
				&mappedMemory));

			std::memcpy(mappedMemory, dataToCopy, static_cast<size_t>(dataSize));

			VkMappedMemoryRange modifiedRange{};
			modifiedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			modifiedRange.pNext = nullptr;
			modifiedRange.memory = memoryObject;
			modifiedRange.offset = offset;
			modifiedRange.size = dataSize;

			std::vector<VkMappedMemoryRange> modifiedRanges = {
				modifiedRange
			};

			VK_CHECK_RESULT(vkFlushMappedMemoryRanges(device, static_cast<uint32_t>(modifiedRanges.size()),
				modifiedRanges.data()));

			vkUnmapMemory(device, memoryObject);
		}

		void CopyDataBetweenBuffers(VkCommandBuffer cmdBuf, VkBuffer srcBuffer, VkBuffer dstBuffer)
		{
			//command buffer must be in record state

			//if we want proceed some transfer operations we must setup buffer barrier!!
			VkBufferCopy region{};
			region.srcOffset = 0;
			region.size = VK_WHOLE_SIZE;
			region.dstOffset = 0;

			std::vector<VkBufferCopy> regions = { region };

			vkCmdCopyBuffer(cmdBuf, srcBuffer, dstBuffer, static_cast<uint32_t>(regions.size()),
				regions.data());

			//end command buffer record state if need
		}

		void CopyDataFromBufferToImage(VkCommandBuffer cmdBuf, VkBuffer srcBuffer,
			VkImage dstImage, VkImageLayout currentImageLayout,
			std::vector<VkBufferImageCopy>& copyRegions)
		{
			vkCmdCopyBufferToImage(cmdBuf, srcBuffer, dstImage, currentImageLayout,
				static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
		}

		void CopyDataFromImageToBuffer(VkCommandBuffer cmdBuf, VkImage srcImage,
			VkBuffer dstBuffer, VkImageLayout currentImageLayout,
			std::vector<VkBufferImageCopy>& copyRegions)
		{
			vkCmdCopyImageToBuffer(cmdBuf, srcImage, currentImageLayout, dstBuffer,
				static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
		}

		void UpdateImageUsingStagingBuffer(VkDevice device, VkPhysicalDevice gpu, 
			void *dataToCopy, VkDeviceSize copySize, VkCommandBuffer cmdBuf, VkImage dstImage, 
			VkQueue queue, std::vector<VkSemaphore>& semaphores,
			VkAccessFlags dstImageCurrentAccessFlags, VkAccessFlags dstImageNewAccessFlags,
			VkImageLayout dstImageCurrentImageLayout, VkImageLayout dstImageNewImageLayout,
			VkPipelineStageFlags dstImageCurrentPipelineFlags, VkPipelineStageFlags dstImageNewPipelineFlags,
			VkImageAspectFlags aspectFlags, VkOffset3D dstImageOffset,
			VkExtent3D dstImageSize, VkImageSubresourceLayers dstSubresources)
		{
			VkBuffer stagingBuffer = CreateBuffer(device, copySize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);
			VkDeviceMemory stageDeviceMemory = AllocateBufferMemory(gpu, device,
				stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			MapUpdateAndUnmapHostVisibleMemory(device, stageDeviceMemory, 0, copySize, dataToCopy);

			BeginCommandBufferRecord(cmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			ImageTransition transitionBefore = SetupImageTransition(dstImage, dstImageCurrentAccessFlags,
				VK_ACCESS_TRANSFER_WRITE_BIT, dstImageCurrentImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, aspectFlags);
			VkImageMemoryBarrier beforeBarrier = SetupImageMemoryBarrier(transitionBefore);
			RecordImageTransition(cmdBuf, beforeBarrier, dstImageCurrentPipelineFlags,
				VK_PIPELINE_STAGE_TRANSFER_BIT);
			CopyDataFromBufferToImage(cmdBuf, stagingBuffer, dstImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { {0, 0, 0, dstSubresources, dstImageOffset, dstImageSize} });
			ImageTransition transitionAfter = SetupImageTransition(dstImage, VK_ACCESS_TRANSFER_WRITE_BIT,
				dstImageNewAccessFlags, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImageNewImageLayout,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, aspectFlags);
			VkImageMemoryBarrier afterBarrier = SetupImageMemoryBarrier(transitionAfter);
			RecordImageTransition(cmdBuf, afterBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT, dstImageNewPipelineFlags);
			EndCommandBufferRecord(cmdBuf);

			VkFence fence = CreateFence(device, 0);

			SubmitCommandBuffersToQueue(queue, {}, { cmdBuf }, semaphores, fence);
			WaitForFences(device, { fence }, VK_FALSE, 500000000);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stageDeviceMemory, nullptr);
		}

		void UpdateBufferUsingStagingBuffer(VkDevice device, VkPhysicalDevice gpu,
			void* dataToCopy, VkDeviceSize dataSize, VkCommandBuffer cmdBuf, VkBuffer dstBuffer,
			VkAccessFlags dstBufferCurrentAccess, VkAccessFlags dstBufferNewAccess,
			VkPipelineStageFlags dstBufferCurrentFlags, VkPipelineStageFlags dstBufferNewFlags,
			VkQueue queue, std::vector<VkSemaphore>& semaphores)
		{
			VkBuffer stagingBuffer = CreateBuffer(device, dataSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);
			VkDeviceMemory stageDeviceMemory = AllocateBufferMemory(gpu, device,
				stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			MapUpdateAndUnmapHostVisibleMemory(device, stageDeviceMemory, 0,
				dataSize, dataToCopy);

			BeginCommandBufferRecord(cmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			BufferTransition transitionBefore{};
			SetupBufferTransition(transitionBefore, dstBuffer, dstBufferCurrentAccess,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
			VkBufferMemoryBarrier dstBufferBarrierBefore = SetupBufferMemoryBarrier(transitionBefore,
				dataSize, 0);
			RecordBufferTransition(cmdBuf, dstBufferBarrierBefore, dstBufferCurrentFlags,
				VK_PIPELINE_STAGE_TRANSFER_BIT);
			CopyDataBetweenBuffers(cmdBuf, stagingBuffer, dstBuffer);
			BufferTransition transitionAfter{};
			SetupBufferTransition(transitionAfter, dstBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, dstBufferNewAccess,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
			VkBufferMemoryBarrier dstBufferBarrierAfter = SetupBufferMemoryBarrier(transitionAfter,
				dataSize, 0);
			RecordBufferTransition(cmdBuf, dstBufferBarrierAfter, VK_PIPELINE_STAGE_TRANSFER_BIT,
				dstBufferNewFlags);
			EndCommandBufferRecord(cmdBuf);

			VkFence fence = CreateFence(device, 0);

			SubmitCommandBuffersToQueue(queue, {}, { cmdBuf }, semaphores,
				fence);

			WaitForFences(device, { fence }, VK_FALSE, 500000000);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stageDeviceMemory, nullptr);
		}

		void DestroyImageView(VkDevice device, VkImageView view)
		{
			vkDestroyImageView(device, view, nullptr);
			//after we can destroy image itself
		}

		void DestroyImage(VkDevice device, VkImage image)
		{
			//before that must destroy image view
			vkDestroyImage(device, image, nullptr);
		}

		void DestroyBufferView(VkDevice device, VkBufferView view)
		{
			vkDestroyBufferView(device, view, nullptr);
			//after we can destroy buffer itself
		}

		void DestroyBuffer(VkDevice device, VkBuffer buffer)
		{
			//before that must destroy buffer view
			vkDestroyBuffer(device, buffer, nullptr);
		}

		void FreeMemoryObject(VkDevice device, VkDeviceMemory obj)
		{
			//use it after all resource objects was destroyed
			vkFreeMemory(device, obj, nullptr);
		}
	};
}