#pragma once

#include <fstream>
#include <thread>
#include <array>

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
		void QuerryAvailableLayers();
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

		VkDeviceMemory AllocateImageMemory(VkPhysicalDevice gpu, VkDevice device, VkImage image, VkMemoryPropertyFlagBits flags)
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
			return deviceMem;
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

		struct Buffer
		{
			VkBuffer buffer;
			VkBufferView bufferView;
		};

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
			void* dataToCopy, VkDeviceSize copySize, VkCommandBuffer cmdBuf, VkImage dstImage,
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
			VkBufferImageCopy imageCopy = {
				0, 
				0,
				0,
				dstSubresources, 
				dstImageOffset,
				dstImageSize
			};

			std::vector<VkBufferImageCopy> copies = { imageCopy };

			CopyDataFromBufferToImage(cmdBuf, stagingBuffer, dstImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copies);
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

		//descriptor sets
		VkSampler CreateSampler(VkDevice device, VkFilter minFilterMode, VkFilter magFilterMode,
			VkSamplerMipmapMode samplerMipMapMode, VkSamplerAddressMode uAdressMode, VkSamplerAddressMode vAdressMode,
			VkSamplerAddressMode wAdressMode, float mipLodBias, bool enableAniso, float maxAniso,
			bool enableReferenceCompare, VkCompareOp compareOp, float minLod, float maxLod,
			VkBorderColor borderColor, bool normalizeCoordinates)
		{
			VkSamplerCreateInfo samplerCI{};
			samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCI.pNext = nullptr;
			samplerCI.flags = 0;
			samplerCI.minFilter = minFilterMode;
			samplerCI.magFilter = magFilterMode;
			samplerCI.mipmapMode = samplerMipMapMode;
			samplerCI.addressModeU = uAdressMode;
			samplerCI.addressModeV = vAdressMode;
			samplerCI.addressModeW = wAdressMode;
			samplerCI.mipLodBias = mipLodBias;
			samplerCI.anisotropyEnable = enableAniso;
			samplerCI.maxAnisotropy = maxAniso;
			samplerCI.compareEnable = enableReferenceCompare;
			samplerCI.compareOp = compareOp;
			samplerCI.minLod = minLod;
			samplerCI.maxLod = maxLod;
			samplerCI.borderColor = borderColor;
			samplerCI.unnormalizedCoordinates = normalizeCoordinates;

			VkSampler sampler;
			VK_CHECK_RESULT(vkCreateSampler(device, &samplerCI, nullptr, &sampler));
			return sampler;
		}

		Image CreateSampledImage(VkDevice device, VkPhysicalDevice gpu, VkFormat imageFormat,
			VkFormatFeatureFlags desiredFeature, VkImageType imageType, VkExtent3D imageSize,
			uint32_t imageLayers, uint32_t mipmapLevels, VkSampleCountFlagBits samplesCount,
			VkImageViewType viewType, VkImageAspectFlags aspectFlags)
		{
			VkFormatProperties formatProps{};
			vkGetPhysicalDeviceFormatProperties(gpu, imageFormat, &formatProps);

			if (!(formatProps.optimalTilingFeatures & desiredFeature))
			{
				LOG_INFO("Type desiredFeature isnt supported");
			}

			VkImage image = CreateImage(device, imageType, imageFormat, imageSize,
				imageLayers, mipmapLevels, samplesCount, VK_IMAGE_USAGE_SAMPLED_BIT);
			VkDeviceMemory imageMemoryObject = AllocateImageMemory(gpu, device, image,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VkImageView view = CreateImageView(device, image, viewType, imageFormat, aspectFlags);
			return { image, view };
		}

		Image CreateCombinedImageSampler()
		{
			//VkSampler sampler = CreateSampler();
			//Image image = CreateSampledImage();
			return {};
		}

		Image CreateStorageImage(VkDevice device, VkPhysicalDevice gpu, VkFormat imageFormat,
			VkImageType imageType, VkExtent3D size, uint32_t mipmapLevels,
			uint32_t layers, VkSampleCountFlagBits samples, VkImageViewType viewType,
			VkImageAspectFlags aspectMask)
		{
			VkFormatProperties imageFormatProps;
			vkGetPhysicalDeviceFormatProperties(gpu, imageFormat, &imageFormatProps);


			if (!(imageFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
			{
				LOG_INFO("Type desiredFeature isnt supported");
			}

			VkImage image = CreateImage(device, imageType, imageFormat, size,
				mipmapLevels, layers, samples, VK_IMAGE_USAGE_STORAGE_BIT);

			VkDeviceMemory imageMemory = AllocateImageMemory(gpu, device,
				image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkImageView view = CreateImageView(device, image, viewType, imageFormat, aspectMask);

			return { image, view };
		}

		Buffer CreateUniformTexelBuffer(VkDevice device, VkPhysicalDevice gpu, VkFormat bufferFormat,
			VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags)
		{
			VkFormatProperties bufferFormatProps;
			vkGetPhysicalDeviceFormatProperties(gpu, bufferFormat, &bufferFormatProps);

			if (!(bufferFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
			{
				LOG_INFO("Type desiredFeature isnt supported");
			}

			VkBuffer buffer = CreateBuffer(device, size, bufferUsageFlags | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE);
			VkDeviceMemory bufferMemory = AllocateBufferMemory(gpu, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VkBufferView view = CreateBufferView(device, buffer, bufferFormat, size, 0);
			return { buffer, view };
		}

		Buffer CreateStorageTexelBuffer(VkDevice device, VkPhysicalDevice gpu, VkFormat bufferFormat,
			VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags)
		{
			VkFormatProperties bufferFormatProps;
			vkGetPhysicalDeviceFormatProperties(gpu, bufferFormat, &bufferFormatProps);

			if (!(bufferFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
			{
				LOG_INFO("Type desiredFeature isnt supported");
			}

			VkBuffer buffer = CreateBuffer(device, size, bufferUsageFlags | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE);
			VkDeviceMemory bufferMemory = AllocateBufferMemory(gpu, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VkBufferView view = CreateBufferView(device, buffer, bufferFormat, size, 0);
			return { buffer, view };
		}

		VkBuffer CreateUniformBuffer(VkDevice device, VkPhysicalDevice gpu,
			VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags)
		{
			VkBuffer buffer = CreateBuffer(device, size, bufferUsageFlags | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE);
			VkDeviceMemory bufferMemory = AllocateBufferMemory(gpu, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return buffer;
		}

		VkBuffer CreateStorageBuffer(VkDevice device, VkPhysicalDevice gpu, VkFormat bufferFormat,
			VkDeviceSize size, VkBufferUsageFlags bufferUsageFlags)
		{
			VkBuffer buffer = CreateBuffer(device, size, bufferUsageFlags | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE);
			VkDeviceMemory bufferMemory = AllocateBufferMemory(gpu, device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			return buffer;
		}

		Image CreateInputAttachment(VkDevice device, VkPhysicalDevice gpu, VkFormat imageFormat,
			VkImageType imageType, VkExtent3D size, uint32_t mipmapLevels,
			uint32_t layers, VkSampleCountFlagBits samples, VkImageViewType viewType,
			VkImageAspectFlags aspectMask, VkFormatFeatureFlags desiredInputType, VkImageUsageFlags imageUsage)
		{
			VkFormatProperties imageFormatProps;
			vkGetPhysicalDeviceFormatProperties(gpu, imageFormat, &imageFormatProps);

			if (!(imageFormatProps.optimalTilingFeatures & desiredInputType))
			{
				LOG_INFO("Type desiredFeature isnt supported");
			}

			VkImage image = CreateImage(device, imageType, imageFormat, size,
				mipmapLevels, layers, samples, imageUsage);

			VkDeviceMemory imageMemory = AllocateImageMemory(gpu, device,
				image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkImageView view = CreateImageView(device, image, viewType, imageFormat, aspectMask);

			return { image, view };
		}

		VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings)
		{
			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.bindingCount = static_cast<uint32_t>(bindings.size());
			info.pBindings = bindings.data();

			VkDescriptorSetLayout layout;
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &info, nullptr, &layout));
			return layout;
		}

		VkDescriptorPool CreateDescriptorPool(VkDevice device, VkDescriptorPoolCreateFlags flags,
			uint32_t maxSets, std::vector<VkDescriptorPoolSize> descriptorTypes)
		{
			VkDescriptorPoolCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = flags;
			info.maxSets = maxSets;
			info.poolSizeCount = static_cast<uint32_t>(descriptorTypes.size());
			info.pPoolSizes = descriptorTypes.data();

			VkDescriptorPool pool;
			VK_CHECK_RESULT(vkCreateDescriptorPool(device, &info, nullptr, &pool));
			return pool;
		}

		std::vector<VkDescriptorSet> AllocateDescriptorSet(VkDevice device, VkDescriptorPool pool, std::vector<VkDescriptorSetLayout> layouts)
		{
			VkDescriptorSetAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.descriptorPool = pool;
			info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
			info.pSetLayouts = layouts.data();

			std::vector<VkDescriptorSet> sets(layouts.size());
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &info, sets.data()));
			return sets;
		}

		struct ImageDescriptorInfo
		{
			VkDescriptorSet descriptorSet;
			uint32_t binding;
			uint32_t arrayElement;
			VkDescriptorType descriptorType;
			std::vector<VkDescriptorImageInfo> imageInfo;
		};

		struct BufferDescriptorInfo
		{
			VkDescriptorSet descriptorSet;
			uint32_t binding;
			uint32_t arrayElement;
			VkDescriptorType descriptorType;
			std::vector<VkDescriptorBufferInfo> bufferInfo;
		};

		struct TexelBufferDescriptorInfo
		{
			VkDescriptorSet descriptorSet;
			uint32_t binding;
			uint32_t arrayElement;
			VkDescriptorType descriptorType;
			std::vector<VkBufferView> bufferInfo;
		};

		struct CopyDescriptorInfo
		{
			VkDescriptorSet srcDescriptorSet;
			uint32_t srcBinding;
			uint32_t srcArrayElement;
			VkDescriptorSet dstDescriptorSet;
			uint32_t dstBinding;
			uint32_t dstArrayElement;
			uint32_t descriptorsCount;
		};

		void UpdateDecsriptorSets(VkDevice device, std::vector<ImageDescriptorInfo>& imageDescriptorsInfo,
			std::vector<BufferDescriptorInfo>& bufferDescriptorsInfo,
			std::vector<TexelBufferDescriptorInfo>& texelBufferDescriptorsInfo,
			std::vector<CopyDescriptorInfo>& copyDescriptorsInfo)
		{
			std::vector<VkWriteDescriptorSet> writeDescriptors;
			std::vector<VkCopyDescriptorSet> copyDescriptors;

			for (auto& imageDescriptor : imageDescriptorsInfo)
			{
				writeDescriptors.push_back({
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					imageDescriptor.descriptorSet,
					imageDescriptor.binding,
					imageDescriptor.arrayElement,
					static_cast<uint32_t>(imageDescriptor.imageInfo.size()),
					imageDescriptor.descriptorType,
					imageDescriptor.imageInfo.data(),
					nullptr,
					nullptr
					});
			}

			for (auto& bufferDescriptor : bufferDescriptorsInfo)
			{
				writeDescriptors.push_back({
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					bufferDescriptor.descriptorSet,
					bufferDescriptor.binding,
					bufferDescriptor.arrayElement,
					static_cast<uint32_t>(bufferDescriptor.bufferInfo.size()),
					bufferDescriptor.descriptorType,
					nullptr,
					bufferDescriptor.bufferInfo.data(),
					nullptr
					});
			}

			for (auto& texelDescriptor : texelBufferDescriptorsInfo)
			{
				writeDescriptors.push_back({
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					texelDescriptor.descriptorSet,
					texelDescriptor.binding,
					texelDescriptor.arrayElement,
					static_cast<uint32_t>(texelDescriptor.bufferInfo.size()),
					texelDescriptor.descriptorType,
					nullptr,
					nullptr,
					texelDescriptor.bufferInfo.data(),
					});
			}

			for (auto& cpyInfo : copyDescriptorsInfo)
			{
				copyDescriptors.push_back({
					VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
					nullptr,
					cpyInfo.srcDescriptorSet,
					cpyInfo.srcBinding,
					cpyInfo.srcArrayElement,
					cpyInfo.dstDescriptorSet,
					cpyInfo.dstBinding,
					cpyInfo.dstArrayElement,
					cpyInfo.descriptorsCount
					});
			}

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(),
				static_cast<uint32_t>(copyDescriptors.size()), copyDescriptors.data());
		}

		void BindDescriptorSets(VkCommandBuffer cmdBuf, VkPipelineBindPoint pipelineType,
			VkPipelineLayout layout, std::vector<VkDescriptorSet>  sets, uint32_t indexForFirstSet)
		{
			BeginCommandBufferRecord(cmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			vkCmdBindDescriptorSets(cmdBuf, pipelineType, layout, indexForFirstSet, static_cast<uint32_t>(sets.size()),
				sets.data(), 0, nullptr);
			EndCommandBufferRecord(cmdBuf);
		}

		void CreateDescriptorsWithTextureAndUniformBuffer(VkDevice device, VkPhysicalDevice gpu, VkExtent3D sampledImageSize,
			VkDeviceSize bufferSize)
		{
			VkSampler sampler = CreateSampler(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,
				VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
				0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
				false);
			Image sampledImage = CreateSampledImage(device, gpu, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
				VK_IMAGE_TYPE_2D, sampledImageSize, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

			VkBuffer uniformBuffer = CreateUniformBuffer(device, gpu, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

			std::vector<VkDescriptorSetLayoutBinding> bindings =
			{
				{
					0,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
				},
				{
					1,
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					1,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
				}
			};

			VkDescriptorSetLayout layout = CreateDescriptorSetLayout(device, bindings);

			std::vector<VkDescriptorPoolSize> descriptorTypes =
			{
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
			};

			VkDescriptorPool pool = CreateDescriptorPool(device, 0, 1, descriptorTypes);
			std::vector<VkDescriptorSet> descriptorSets = AllocateDescriptorSet(device, pool, { layout });

			ImageDescriptorInfo samplerInfo{};
			samplerInfo.descriptorSet = descriptorSets[0];
			samplerInfo.binding = 0;
			samplerInfo.arrayElement = 0;
			samplerInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			VkDescriptorImageInfo imageInfo{};
			imageInfo.sampler = sampler;
			imageInfo.imageView = sampledImage.view;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			samplerInfo.imageInfo = { imageInfo };

			BufferDescriptorInfo bufferInfo{};
			bufferInfo.descriptorSet = descriptorSets[1];
			bufferInfo.binding = 1;
			bufferInfo.arrayElement = 0;
			bufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			VkDescriptorBufferInfo bufferDI{};
			bufferDI.buffer = uniformBuffer;
			bufferDI.offset = 0;
			bufferDI.range = VK_WHOLE_SIZE;
			bufferInfo.bufferInfo = { bufferDI };

			std::vector<ImageDescriptorInfo> imageInfos = { samplerInfo };
			std::vector<BufferDescriptorInfo> bufferInfos = { bufferInfo };
			std::vector<TexelBufferDescriptorInfo> texelBufferDescriptorsInfo;
			std::vector<CopyDescriptorInfo> copyDescriptorsInfo;
			UpdateDecsriptorSets(device, imageInfos, bufferInfos,
				texelBufferDescriptorsInfo, copyDescriptorsInfo);
		}

		void FreeDescriptorSets(VkDevice device, VkDescriptorPool pool, std::vector<VkDescriptorSet> sets)
		{
			VK_CHECK_RESULT(vkFreeDescriptorSets(device, pool, static_cast<uint32_t>(sets.size()), sets.data()));
			sets.clear();
		}

		void ResetDescriptorPool(VkDevice device, VkDescriptorPool pool)
		{
			VK_CHECK_RESULT(vkResetDescriptorPool(device, pool, 0));
		}

		void DesctroyDescriptorPool(VkDevice device, VkDescriptorPool pool)
		{
			vkDestroyDescriptorPool(device, pool, nullptr);
		}

		void DestroyDescroptorSetLayout(VkDevice device, VkDescriptorSetLayout layout)
		{
			vkDestroyDescriptorSetLayout(device, layout, nullptr);
		}

		void DestroySampler(VkDevice device, VkSampler sampler)
		{
			vkDestroySampler(device, sampler, nullptr);
		}

		//render passes
		VkAttachmentDescription SpecifyColorAttachmentDescription()
		{
			return {
				0,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			};
		}

		VkAttachmentDescription SpecifyDepthAttachmentDescription()
		{
			return {
				0,
				VK_FORMAT_D16_UNORM,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};
		}

		struct SubpassParams
		{
			VkPipelineBindPoint pipelineType;
			std::vector<VkAttachmentReference> inputAttachments;
			std::vector<VkAttachmentReference> colorAttachments;
			std::vector<VkAttachmentReference> resolveAttachments;
			VkAttachmentReference depthStencilAttachment;
			std::vector<uint32_t> preserveAttachments;
		};

		std::vector<VkSubpassDescription> SpecifySubpassDescriptions(std::vector<SubpassParams>& params)
		{
			std::vector<VkSubpassDescription> descriptions(params.size());
			int index = 0;
			for (auto& descriptor : descriptions)
			{
				descriptor.flags = 0;
				descriptor.pipelineBindPoint = params[index].pipelineType;

				descriptor.inputAttachmentCount = static_cast<uint32_t>(params[index].inputAttachments.size());
				descriptor.pInputAttachments = params[index].inputAttachments.data();

				descriptor.colorAttachmentCount = static_cast<uint32_t>(params[index].colorAttachments.size());
				descriptor.pColorAttachments = params[index].colorAttachments.data();

				descriptor.pResolveAttachments = params[index].resolveAttachments.data();
				descriptor.pDepthStencilAttachment = &params[index].depthStencilAttachment;

				descriptor.preserveAttachmentCount = static_cast<uint32_t>(params[index].preserveAttachments.size());
				descriptor.pPreserveAttachments = params[index].preserveAttachments.data();

				index++;
			}
			return descriptions;
		}

		void SpecifyDependencyBetweenSubpasses()
		{
			//p 299(329) from CookBook
			//exampe
			std::vector<VkSubpassDependency> subpassDependencies = {
				{
					0,
					1,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT
				},
			};
		}

		VkRenderPass CreateRenderPass(VkDevice device,
			std::vector<VkAttachmentDescription> attachmentDescriptions,
			std::vector<VkSubpassDescription> subpassDescriptions,
			std::vector<VkSubpassDependency> subpassDependencies)
		{
			VkRenderPassCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
			info.pAttachments = attachmentDescriptions.data();
			info.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
			info.pDependencies = subpassDependencies.data();
			info.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
			info.pSubpasses = subpassDescriptions.data();

			VkRenderPass pass;
			VK_CHECK_RESULT(vkCreateRenderPass(device, &info, nullptr, &pass));
			return pass;
		}

		VkFramebuffer CreateFramebuffer(VkDevice device, VkRenderPass renderPass, std::vector<VkImageView> attachments,
			VkExtent2D framebufferSize, uint32_t layersCount)
		{
			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.renderPass = renderPass;
			info.attachmentCount = static_cast<uint32_t>(attachments.size());
			info.pAttachments = attachments.data();
			info.width = framebufferSize.width;
			info.height = framebufferSize.height;
			info.layers = layersCount;

			VkFramebuffer buffer;
			VK_CHECK_RESULT(vkCreateFramebuffer(device, &info, nullptr, &buffer));
			return buffer;
		}

		//exercises
		void PrepareRenderPassForGeometryRenderAndPostProcessSubpass(VkDevice device)
		{
			std::vector<VkAttachmentDescription> attachmentsDescriptions(3);
			//color
			attachmentsDescriptions[0].flags = 0;
			attachmentsDescriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;
			attachmentsDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentsDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentsDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentsDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentsDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentsDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentsDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			//color
			attachmentsDescriptions[1].flags = 0;
			attachmentsDescriptions[1].format = VK_FORMAT_D16_UNORM;
			attachmentsDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentsDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentsDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentsDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentsDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentsDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentsDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			//color
			attachmentsDescriptions[2].flags = 0;
			attachmentsDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
			attachmentsDescriptions[2].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentsDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentsDescriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentsDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentsDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentsDescriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentsDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference depthStencilReference = {
				1,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};

			std::vector<SubpassParams> subpassParams(2);
			subpassParams[0].pipelineType = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassParams[0].inputAttachments = {};
			subpassParams[0].colorAttachments = {
				{
					0,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}
			};
			subpassParams[0].resolveAttachments = {};
			subpassParams[0].depthStencilAttachment = depthStencilReference;
			subpassParams[0].preserveAttachments = {};

			subpassParams[1].pipelineType = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassParams[1].inputAttachments = {
				{
					2,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				}
			};
			subpassParams[1].colorAttachments = {
				{
					0,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}
			};
			subpassParams[1].resolveAttachments = {};
			subpassParams[1].depthStencilAttachment = {};
			subpassParams[1].preserveAttachments = {};

			std::vector<VkSubpassDependency> subpassDeps =
			{
				{
					0,
					1,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT
				}
			};

			std::vector<VkSubpassDescription> deps = SpecifySubpassDescriptions(subpassParams);
			VkRenderPass pass = CreateRenderPass(device, attachmentsDescriptions, deps, subpassDeps);
		}

		void PrepareRenderPassAndFramebufferWithColorAndDepthAttachments(VkDevice device, VkPhysicalDevice gpu,
			VkExtent3D imageSize)
		{
			VkImage colorImage = CreateImage(device, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
				imageSize, 0, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
			VkDeviceMemory colorImageMemory = AllocateImageMemory(gpu, device, colorImage,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VkImageView colorImageView = CreateImageView(device, colorImage, VK_IMAGE_VIEW_TYPE_2D,
				VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

			VkImage depthImage = CreateImage(device, VK_IMAGE_TYPE_2D, VK_FORMAT_D16_UNORM,
				imageSize, 0, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			VkDeviceMemory depthImageMemory = AllocateImageMemory(gpu, device, colorImage,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VkImageView depthImageView = CreateImageView(device, colorImage, VK_IMAGE_VIEW_TYPE_2D,
				VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT);

			std::vector<VkAttachmentDescription> attachmentsDescriptions(3);
			//color
			attachmentsDescriptions[0].flags = 0;
			attachmentsDescriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;
			attachmentsDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentsDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentsDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentsDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentsDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentsDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentsDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			//color
			attachmentsDescriptions[1].flags = 0;
			attachmentsDescriptions[1].format = VK_FORMAT_D16_UNORM;
			attachmentsDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentsDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentsDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentsDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentsDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentsDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentsDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthStencilReference = {
				1,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};

			std::vector<SubpassParams> subpassParams(1);
			subpassParams[0].pipelineType = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassParams[0].inputAttachments = {};
			subpassParams[0].colorAttachments = {
				{
					0,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				}
			};
			subpassParams[0].resolveAttachments = {};
			subpassParams[0].depthStencilAttachment = depthStencilReference;
			subpassParams[0].preserveAttachments = {};

			std::vector<VkSubpassDescription> deps = SpecifySubpassDescriptions(subpassParams);

			std::vector<VkSubpassDependency> subpassDeps =
			{
				{
					0,
					VK_SUBPASS_EXTERNAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					0
				}
			};

			VkRenderPass pass = CreateRenderPass(device, attachmentsDescriptions, deps, subpassDeps);
			VkFramebuffer buffer = CreateFramebuffer(device, pass,
				{ colorImageView, depthImageView },
				{ imageSize.width, imageSize.height }, 1);
		}

		void BeginRenderPass(VkRenderPass renderPass,
			VkCommandBuffer cmdBuf, VkFramebuffer framebuffer, VkRect2D renderArea, std::vector<VkClearValue> clearValues,
			VkSubpassContents subpassContents)
		{
			VkRenderPassBeginInfo info{};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.clearValueCount = static_cast<uint32_t>(clearValues.size());
			info.pClearValues = clearValues.data();
			info.framebuffer = framebuffer;
			info.renderArea = renderArea;
			info.renderPass = renderPass;

			//cmd buf must be in record state

			vkCmdBeginRenderPass(cmdBuf, &info, subpassContents);

		}

		void ProcessToNextSubpass(VkCommandBuffer cmdBuf, VkSubpassContents contents)
		{
			vkCmdNextSubpass(cmdBuf, contents);
		}

		void EndRenderPass(VkCommandBuffer cmdBuf)
		{
			vkCmdEndRenderPass(cmdBuf);
		}

		void DestroyFramebuffer(VkDevice device, VkFramebuffer buffer)
		{
			vkDestroyFramebuffer(device, buffer, nullptr);
		}

		void DestroyRenderpass(VkDevice device, VkRenderPass pass)
		{
			vkDestroyRenderPass(device, pass, nullptr);
		}

		//graphics and compute pipeline
		VkShaderModule CreateShaderModule(VkDevice device, std::vector<unsigned char> sourceCode)
		{
			VkShaderModuleCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.codeSize = sourceCode.size();
			info.pCode = reinterpret_cast<uint32_t const*> (sourceCode.data());

			VkShaderModule shaderModule{};
			VK_CHECK_RESULT(vkCreateShaderModule(device, &info, nullptr, &shaderModule));
			return shaderModule;
		}

		struct ShaderStageParams
		{
			VkShaderStageFlagBits shaderStage;
			VkShaderModule shaderModule;
			char const* entryPoint;
			VkSpecializationInfo const* specialisationInfo;
		};

		std::vector<VkPipelineShaderStageCreateInfo>& SpecifyPipelineShaderStages(
			std::vector<ShaderStageParams> shaderStageParams)
		{
			std::vector<VkPipelineShaderStageCreateInfo> infos(shaderStageParams.size());
			for (auto& param : shaderStageParams)
			{
				VkPipelineShaderStageCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				info.pNext = nullptr;
				info.flags = 0;
				info.module = param.shaderModule;
				info.pName = param.entryPoint;
				info.pSpecializationInfo = param.specialisationInfo;
				info.stage = param.shaderStage;
				infos.push_back(info);
			}
			return infos;
		}

		VkPipelineVertexInputStateCreateInfo SpecifyBindingAttributeDescriptions(std::vector<VkVertexInputBindingDescription> bindDescriptions,
			std::vector<VkVertexInputAttributeDescription> attribDescriptions)
		{
			VkPipelineVertexInputStateCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.vertexBindingDescriptionCount = static_cast<uint32_t>(bindDescriptions.size());
			info.pVertexBindingDescriptions = bindDescriptions.data();
			info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescriptions.size());
			info.pVertexAttributeDescriptions = attribDescriptions.data();
			return info;
		}

		VkPipelineInputAssemblyStateCreateInfo SpecifyPipelineInputAssemblyState(VkBool32 primitiveRestartEnable,
			VkPrimitiveTopology topology)
		{
			VkPipelineInputAssemblyStateCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.primitiveRestartEnable = primitiveRestartEnable;
			info.topology = topology;
		}

		VkPipelineTessellationStateCreateInfo SpecifyTessState(uint32_t patchesCout)
		{
			VkPipelineTessellationStateCreateInfo info = {
				VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
				nullptr,
				0,
				patchesCout
			};
			return info;
		}

		struct ViewportInfo
		{
			std::vector<VkViewport> viewPorts;
			std::vector<VkRect2D> scissors;
		};

		VkPipelineViewportStateCreateInfo SpecifyViewportAndScissorsState(ViewportInfo viewportInfo)
		{
			VkPipelineViewportStateCreateInfo info =
			{
				VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				nullptr,
				0,
				static_cast<uint32_t>(viewportInfo.viewPorts.size()),
				viewportInfo.viewPorts.data(),
				static_cast<uint32_t>(viewportInfo.scissors.size()),
				viewportInfo.scissors.data(),
			};
			return info;
		}

		VkPipelineRasterizationStateCreateInfo SpecifyRasterizationState(VkBool32 depthClampEnable,
			VkBool32 rasterizerDiscardEnabled, VkPolygonMode polygonMode, VkCullModeFlags cullMode,
			VkFrontFace frontFace, VkBool32 depthBiasEnabled, float depthBiasFactor,
			float depthBiasClamp, float depthBiasSlope, float lineWidth)
		{
			VkPipelineRasterizationStateCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.depthClampEnable = depthClampEnable;
			info.rasterizerDiscardEnable = rasterizerDiscardEnabled;
			info.polygonMode = polygonMode;
			info.cullMode = cullMode;
			info.frontFace = frontFace;
			info.depthBiasEnable = depthBiasEnabled;
			info.depthBiasConstantFactor = depthBiasFactor;
			info.depthBiasClamp = depthBiasClamp;
			info.depthBiasSlopeFactor = depthBiasSlope;
			info.lineWidth = lineWidth;
			return info;
		}

		VkPipelineMultisampleStateCreateInfo SpecifyMultisamplingState(
			VkSampleCountFlagBits rasterizationSamples,
			VkBool32 sampleShadingEnable,
			float minSampleShading,
			const VkSampleMask* pSampleMask,
			VkBool32 alphaToCoverageEnable,
			VkBool32 alphaToOneEnable)
		{
			VkPipelineMultisampleStateCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.rasterizationSamples = rasterizationSamples;
			info.sampleShadingEnable = sampleShadingEnable;
			info.minSampleShading = minSampleShading;
			info.pSampleMask = pSampleMask;
			info.alphaToCoverageEnable = alphaToCoverageEnable;
			info.alphaToOneEnable = alphaToOneEnable;
			return info;
		}

		VkPipelineDepthStencilStateCreateInfo SpecifyDepthStencilState(
			VkBool32 depthTestEnable,
			VkBool32 depthWriteEnable,
			VkCompareOp depthCompareOp,
			VkBool32 depthBoundsTestEnable,
			VkBool32 stencilTestEnable,
			VkStencilOpState front,
			VkStencilOpState back,
			float minDepthBounds,
			float maxDepthBounds)
		{
			VkPipelineDepthStencilStateCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.depthTestEnable = depthTestEnable;
			info.depthWriteEnable = depthWriteEnable;
			info.depthCompareOp = depthCompareOp;
			info.depthBoundsTestEnable = depthBoundsTestEnable;
			info.stencilTestEnable = stencilTestEnable;
			info.front = front;
			info.back = back;
			info.minDepthBounds = minDepthBounds;
			info.maxDepthBounds = maxDepthBounds;
			return info;
		}

		VkPipelineColorBlendStateCreateInfo SpecifyColorBlendState(
			std::vector< VkPipelineColorBlendAttachmentState> states,
			VkBool32 logicOpEnable,
			VkLogicOp logicOp,
			uint32_t attachmentCount,
			const VkPipelineColorBlendAttachmentState* pAttachments,
			float blendConstants[4])
		{
			VkPipelineColorBlendStateCreateInfo info =
			{
				VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				nullptr,
				0,
				logicOpEnable,
				logicOp,
				attachmentCount,
				pAttachments,
				blendConstants[0]
			};
			return info;
		}

		VkPipelineDynamicStateCreateInfo SpecifyDynamicStates(std::vector<VkDynamicState> dynStates)
		{
			VkPipelineDynamicStateCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.dynamicStateCount = static_cast<uint32_t>(dynStates.size());
			info.pDynamicStates = dynStates.data();
			return info;
		}

		VkPipelineLayout CreatePipelineLayout(VkDevice device,
			std::vector<VkDescriptorSetLayout> layouts,
			std::vector<VkPushConstantRange> pushConstantRange)
		{
			VkPipelineLayoutCreateInfo info =
			{
				VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				nullptr,
				0,
				static_cast<uint32_t>(layouts.size()),
				layouts.data(),
				static_cast<uint32_t>(pushConstantRange.size()),
				pushConstantRange.data()
			};

			VkPipelineLayout layout;
			VK_CHECK_RESULT(vkCreatePipelineLayout(device, &info, nullptr, &layout));
			return layout;
		}

		VkGraphicsPipelineCreateInfo SpecifyGraphicsPipelineCreateInfo(
			VkPipelineCreateFlags additionalOptions,
			std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo,
			VkPipelineVertexInputStateCreateInfo vertexInputStateInfo,
			VkPipelineInputAssemblyStateCreateInfo assemblyInfo,
			VkPipelineTessellationStateCreateInfo tessInfo,
			VkPipelineViewportStateCreateInfo viewportInfo,
			VkPipelineRasterizationStateCreateInfo rasterizationInfo,
			VkPipelineMultisampleStateCreateInfo msInfo,
			VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
			VkPipelineColorBlendStateCreateInfo blendInfo,
			VkPipelineDynamicStateCreateInfo dynamicInfo,
			VkPipelineLayout layout,
			VkRenderPass pass,
			uint32_t subpass,
			VkPipeline parentPipeline,
			int32_t parentIndex)
		{
			VkGraphicsPipelineCreateInfo info = {
				VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				nullptr,
				additionalOptions,
				static_cast<uint32_t>(shaderStagesInfo.size()),
				shaderStagesInfo.data(),
				&vertexInputStateInfo,
				&assemblyInfo,
				&tessInfo,
				&viewportInfo,
				&rasterizationInfo,
				&msInfo,
				&depthStencilInfo,
				&blendInfo,
				&dynamicInfo,
				layout,
				pass,
				subpass,
				parentPipeline,
				parentIndex
			};

			return info;
		}

		VkPipelineCache CreatePipelineCacheObject(VkDevice device, std::vector<unsigned char> cacheData)
		{
			VkPipelineCacheCreateInfo info =
			{
				VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
				nullptr,
				0,
				static_cast<uint32_t>(cacheData.size()),
				reinterpret_cast<const void*>(cacheData.data())
			};

			VkPipelineCache cache;
			VK_CHECK_RESULT(vkCreatePipelineCache(device, &info, nullptr, &cache));
			return cache;
		}

		std::vector<unsigned int>& RetrieveDataFromPipelineCache(VkDevice device, VkPipelineCache cache)
		{
			size_t dataSize;
			std::vector<unsigned int> data;
			VK_CHECK_RESULT(vkGetPipelineCacheData(device, cache, &dataSize, nullptr));
			data.resize(dataSize);
			VK_CHECK_RESULT(vkGetPipelineCacheData(device, cache, &dataSize, data.data()));
			return data;
		}

		VkPipelineCache MergePipelineCaches(VkDevice device, std::vector<VkPipelineCache>& caches)
		{
			VkPipelineCache cache;
			VK_CHECK_RESULT(vkMergePipelineCaches(device, cache,
				static_cast<uint32_t>(caches.size()), caches.data()));
			return cache;
		}

		void CreateGraphicsPipeline(VkDevice device, VkGraphicsPipelineCreateInfo info,
			VkPipelineCache cache, VkPipeline& pipeline)
		{
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, cache, 1, &info, nullptr,
				&pipeline));
		}

		VkPipeline CreateComputePipepline(VkDevice device, VkPipelineCreateFlags additionalOptions,
			VkPipelineShaderStageCreateInfo shaderInfo, VkPipelineLayout layout,
			VkPipelineCache cache, VkPipeline parent, int32_t parentIndex)
		{
			VkComputePipelineCreateInfo info =
			{
				VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				nullptr,
				additionalOptions,
				shaderInfo,
				layout,
				parent,
				parentIndex
			};

			VkPipeline computePipeline;
			VK_CHECK_RESULT(vkCreateComputePipelines(device, cache, 1, &info, nullptr, &computePipeline));
			return computePipeline;
		}

		void BindPipeline(VkCommandBuffer cmdBuf, VkPipeline pipeline, VkPipelineBindPoint type)
		{
			vkCmdBindPipeline(cmdBuf, type, pipeline);
		}

		VkPipelineLayout CreateLayoutWithCombinedImageSamplerBufferPushConstantRanges(VkDevice device,
			std::vector<VkPushConstantRange> pushConstants)
		{
			std::vector<VkDescriptorSetLayoutBinding> layoutBinds(2);
			layoutBinds[0].binding = 0;
			layoutBinds[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			layoutBinds[0].descriptorCount = 1;
			layoutBinds[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBinds[0].pImmutableSamplers = nullptr;

			layoutBinds[1].binding = 1;
			layoutBinds[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinds[1].descriptorCount = 1;
			layoutBinds[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBinds[1].pImmutableSamplers = nullptr;

			VkDescriptorSetLayout layout = CreateDescriptorSetLayout(device, layoutBinds);
			VkPipelineLayout pipelineLayout = CreatePipelineLayout(device, { layout }, pushConstants);
			return pipelineLayout;
		}

		bool GetBinaryFileContents(std::string const& filename,
			std::vector<unsigned char>& contents) {
			contents.clear();

			std::ifstream file(filename, std::ios::binary);
			if (file.fail()) {
				std::cout << "Could not open '" << filename << "' file." << std::endl;
				return false;
			}

			std::streampos begin;
			std::streampos end;
			begin = file.tellg();
			file.seekg(0, std::ios::end);
			end = file.tellg();

			if ((end - begin) == 0) {
				std::cout << "The '" << filename << "' file is empty." << std::endl;
				return false;
			}
			contents.resize(static_cast<size_t>(end - begin));
			file.seekg(0, std::ios::beg);
			file.read(reinterpret_cast<char*>(contents.data()), end - begin);
			file.close();

			return true;
		}


		VkPipeline CreateGraphicsPipeline(VkDevice device,
			VkPipelineCreateFlags additionalOptions,
			std::vector<unsigned char>& vertexCode,
			std::vector<unsigned char> fragmentCode,
			std::vector<VkVertexInputBindingDescription>& bindDescriptions,
			std::vector<VkVertexInputAttributeDescription>& attribDescriptions,
			VkPrimitiveTopology topology,
			VkBool32 assemblyRestartEnable,
			ViewportInfo viewPort, 
			VkPolygonMode polygonMode, 
			VkCullModeFlags cullMode, 
			VkFrontFace frontFace,
			VkPipelineLayout layout, 
			VkRenderPass pass,
			uint32_t subpass)
		{
			VkShaderModule vertexModule = CreateShaderModule(device, vertexCode);
			VkShaderModule fragmentModule = CreateShaderModule(device, fragmentCode);

			std::vector<ShaderStageParams> shaderParams(2);
			shaderParams[0].entryPoint = "main";
			shaderParams[0].shaderModule = vertexModule;
			shaderParams[0].shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderParams[0].specialisationInfo = nullptr;

			shaderParams[0].entryPoint = "main";
			shaderParams[0].shaderModule = fragmentModule;
			shaderParams[0].shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderParams[0].specialisationInfo = nullptr;

			std::vector<VkPipelineShaderStageCreateInfo> shaderStages = SpecifyPipelineShaderStages(shaderParams);
			VkPipelineVertexInputStateCreateInfo inputInfo = SpecifyBindingAttributeDescriptions(bindDescriptions, attribDescriptions);
			VkPipelineInputAssemblyStateCreateInfo assemblyInfo = SpecifyPipelineInputAssemblyState(assemblyRestartEnable, topology);
			VkPipelineViewportStateCreateInfo viewPortInfo = SpecifyViewportAndScissorsState(viewPort);
			VkPipelineRasterizationStateCreateInfo rasterization = SpecifyRasterizationState(false, false,
				polygonMode, cullMode, frontFace, false, 0.0f, 1.0f, 0.0f, 1.0f);
			VkPipelineMultisampleStateCreateInfo msInfo = SpecifyMultisamplingState(VK_SAMPLE_COUNT_1_BIT,
				false, 0.0f, nullptr, false, false);
			VkStencilOpState stencilTest =
			{
				VK_STENCIL_OP_KEEP,
				VK_STENCIL_OP_KEEP,
				VK_STENCIL_OP_KEEP,
				VK_COMPARE_OP_ALWAYS,
				0,
				0,
				0,
			};
			VkPipelineDepthStencilStateCreateInfo depthStencilInfo = SpecifyDepthStencilState(true, true,
				VK_COMPARE_OP_LESS_OR_EQUAL, false, 0.0f, stencilTest, {}, 0.0f, 1.0f);
			std::vector<VkDynamicState> dynamicStates =
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR,
			};
			VkPipelineDynamicStateCreateInfo dynInfo = SpecifyDynamicStates(dynamicStates);
			VkGraphicsPipelineCreateInfo graphicsPipelineInfo = SpecifyGraphicsPipelineCreateInfo(additionalOptions,
				shaderStages, inputInfo, assemblyInfo, {}, viewPortInfo, rasterization, msInfo, depthStencilInfo,
				{}, dynInfo, layout, pass, subpass, {}, -1);

			VkPipeline pipeline;
			CreateGraphicsPipeline(device, graphicsPipelineInfo, {}, pipeline);
			return pipeline;
		}

		void DestroyPipeline(VkDevice device, VkPipeline pipeline)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
		}

		void DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache)
		{
			vkDestroyPipelineCache(device, pipelineCache, nullptr);
		}

		void DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout)
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		}

		void DestroyShadermodule(VkDevice device, VkShaderModule module)
		{
			vkDestroyShaderModule(device, module, nullptr);
		}

		//drawing
		void ClearColorImage(VkCommandBuffer buffer, VkImage image, VkImageLayout layout,
			std::vector<VkImageSubresourceRange> ranges, VkClearColorValue clearColor)
		{
			vkCmdClearColorImage(buffer, image, layout, &clearColor, static_cast<uint32_t>(ranges.size()),
				ranges.data());
		}

		void ClearDepthStencilImage(VkCommandBuffer buffer, VkImage image, VkImageLayout layout,
			std::vector<VkImageSubresourceRange> ranges, VkClearDepthStencilValue clearValues)
		{
			vkCmdClearDepthStencilImage(buffer, image, layout, &clearValues, static_cast<uint32_t>(ranges.size()),
				ranges.data());
		}

		void ClearRenderPassAttachments(VkCommandBuffer buffer, std::vector<VkClearAttachment> attachments,
			std::vector<VkClearRect> rects)
		{
			vkCmdClearAttachments(buffer, static_cast<uint32_t>(attachments.size()),
				attachments.data(), static_cast<uint32_t>(rects.size()), rects.data());;
		}

		struct VertexBufferParams
		{
			VkBuffer buffer;
			VkDeviceSize memoryOffset;
		};

		void BindVertexBuffers(VkCommandBuffer commandBuffer, std::vector<VertexBufferParams> bufferParams,
			uint32_t firstBinding)
		{
			std::vector<VkBuffer> buffers(bufferParams.size());
			std::vector<VkDeviceSize> offsets(bufferParams.size());

			for (auto param : bufferParams)
			{
				buffers.push_back(param.buffer);
				offsets.push_back(param.memoryOffset);
			}

			vkCmdBindVertexBuffers(commandBuffer, firstBinding, static_cast<uint32_t>(bufferParams.size()),
				buffers.data(), offsets.data());
		}

		void BindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer indexBuffer, VkDeviceSize offset, VkIndexType type)
		{
			//befor it, we must bind index buffer using vkCmdBindIndexBuffer();
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, offset, type);
		}

		void PushConstantToShader(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
			VkShaderStageFlags pipelineStages, uint32_t offset, uint32_t size, void* dataToPush)
		{
			vkCmdPushConstants(commandBuffer, pipelineLayout, pipelineStages, offset, size, dataToPush);
		}

		void SetupViewportStateDynamically(VkCommandBuffer commandBuffer, uint32_t firstViewport,
			std::vector<VkViewport> viewports)
		{
			vkCmdSetViewport(commandBuffer, firstViewport, 
				static_cast<uint32_t>(viewports.size()), viewports.data());
		}

		void SetupScissorsStateDynamically(VkCommandBuffer commandBuffer, uint32_t firstScissor,
			std::vector<VkRect2D> scrissors)
		{
			vkCmdSetScissor(commandBuffer, firstScissor,
				static_cast<uint32_t>(scrissors.size()), scrissors.data());
		}

		void SetupLineWidthStateDynamically(VkCommandBuffer commandBuffer, float lineWidth)
		{
			vkCmdSetLineWidth(commandBuffer, lineWidth);
		}

		void SetupDepthBiasStateDynamically(VkCommandBuffer commandBuffer, float constantFactor,
			float clamp, float slopeFactor)
		{
			vkCmdSetDepthBias(commandBuffer, constantFactor, clamp, slopeFactor);
		}

		void SetupBlendConstantsStateDynamically(VkCommandBuffer commandBuffer, std::array<float, 4> blendConstants)
		{
			vkCmdSetBlendConstants(commandBuffer, blendConstants.data());
		}

		void Draw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
			uint32_t firstVertex, uint32_t firstInstance)
		{
			vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void DrawIndexed(VkCommandBuffer buffer, uint32_t indexCount, uint32_t instanceCount,
			uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
		{
			vkCmdDrawIndexed(buffer, indexCount, instanceCount, firstIndex, vertexOffset,
				firstInstance);
		}

		void DispatchComputeWork(VkCommandBuffer buffer, uint32_t xSize, uint32_t ySize, uint32_t zSize)
		{
			vkCmdDispatch(buffer, xSize, ySize, zSize);
		}

		void ExecuteSecondaryCommandBuffer(VkCommandBuffer commandBuffer, std::vector<VkCommandBuffer> secondaryBuffers)
		{
			vkCmdExecuteCommands(commandBuffer, static_cast<uint32_t>(secondaryBuffers.size()), secondaryBuffers.data());
		}

		struct Mesh
		{
			std::vector<float> data;
			std::vector<uint32_t> vertexOffset;
			std::vector<uint32_t> vertexCount;
		};

		//recording command buffer that draws a geometry with 
		//dynamic viewport and scissor states
		void RecordDrawingCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, VkImage swapchainImage,
			uint32_t presentFamilyIndex, uint32_t computeFamilyIndex,
			VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D framebufferSize,
			std::vector<VkClearValue> clearValues, VkPipeline pipeline,
			std::vector<VertexBufferParams> vertexBufferParams, uint32_t firstVertexBufferBind,
			VkPipelineLayout pipelineLayout, std::vector<VkDescriptorSet> sets, 
			uint32_t firstDescriptorSet, Mesh mesh, uint32_t vertexCount, uint32_t instanceCount)
		{
			BeginCommandBufferRecord(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			
			if (presentFamilyIndex != computeFamilyIndex)
			{
				ImageTransition transition = SetupImageTransition(swapchainImage,
					VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					presentFamilyIndex, computeFamilyIndex, VK_IMAGE_ASPECT_COLOR_BIT);
				VkImageMemoryBarrier barrier = SetupImageMemoryBarrier(transition);
				RecordImageTransition(commandBuffer, barrier, 
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			}

			BeginRenderPass(renderPass, commandBuffer, framebuffer, framebufferSize,
				clearValues, VK_SUBPASS_CONTENTS_INLINE);

			BindPipeline(commandBuffer, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

			VkViewport viewPort =
			{
				0.0f,
				0.0f,
				static_cast<float>(framebufferSize.extent.width),
				static_cast<float>(framebufferSize.extent.height),
				0.0f,
				1.0f
			};

			SetupViewportStateDynamically(commandBuffer, 0, { viewPort });
			
			VkRect2D scissor =
			{
				0,
				0,
				framebufferSize.extent.width,
				framebufferSize.extent.height
			};

			SetupScissorsStateDynamically(commandBuffer, 0, { scissor });

			BindVertexBuffers(commandBuffer, vertexBufferParams, firstVertexBufferBind);
			BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
				sets, firstDescriptorSet);

			//setup later from there to draw
			Draw(commandBuffer, vertexCount, instanceCount, 0, 0);
			EndRenderPass(commandBuffer);

			if (presentFamilyIndex != computeFamilyIndex)
			{
				ImageTransition transition = SetupImageTransition(swapchainImage,
					VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					presentFamilyIndex, computeFamilyIndex, VK_IMAGE_ASPECT_COLOR_BIT);
				VkImageMemoryBarrier barrier = SetupImageMemoryBarrier(transition);
				RecordImageTransition(commandBuffer, barrier,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
			}

			EndCommandBufferRecord(commandBuffer);
;		}
	};
}