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
			samplerInfo.imageInfo = { imageInfo } ;

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

			UpdateDecsriptorSets(device, { samplerInfo }, { bufferInfo }, {}, {});

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

		void BeginRenderPass(VkDevice device, VkRenderPass renderPass, 
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
	};
}