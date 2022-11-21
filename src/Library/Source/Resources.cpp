#include "Resources.h"

namespace vk
{
    void CreateBuffer(VkDevice device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& buffer)
    {
        VkBufferCreateInfo bufferInfo =
        {
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            nullptr,
            0,
            size,
            usage,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr
        };

        VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));
    }

    void AllocateAndBindMemoryObjectToBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkBuffer buffer,
        VkMemoryPropertyFlagBits memoryProperties,
        VkDeviceMemory& memoryObject)
    {
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(gpu, &physicalDeviceMemoryProperties);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
                (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties)
            {
                VkMemoryAllocateInfo allocInfo =
                {
                    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    nullptr,
                    memoryRequirements.size,
                    i
                };

                VkResult res = vkAllocateMemory(device, &allocInfo, nullptr, &memoryObject);
                if (res == VK_SUCCESS)
                {
                    break;
                }
            }
        }

        if (memoryObject == VK_NULL_HANDLE)
        {
            WARN_LOG("Failed to create memory object for buffer");
            return;
        }

        VK_CHECK_RESULT(vkBindBufferMemory(device, buffer, memoryObject, 0));
    }

    void SetBufferMemoryBarrier(VkCommandBuffer commandBuffer,
        VkPipelineStageFlags generatingStages,
        VkPipelineStageFlags consumingStages,
        std::vector<BufferTransition> bufferTransitions)
    {
        std::vector<VkBufferMemoryBarrier> memoryBarriers;
        for (auto& transition : bufferTransitions)
        {
            memoryBarriers.push_back(
                {
                    VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    nullptr,
                    transition.srcFlags,
                    transition.dstFlags,
                    transition.srcQueueFamilyIndex,
                    transition.dstQueueFamilyIndex,
                    transition.buffer,
                    0,
                    VK_WHOLE_SIZE
                });
        }

        if (memoryBarriers.size() == 0)
        {
            return;
        }

        vkCmdPipelineBarrier(commandBuffer, generatingStages, consumingStages, 0, 0,
            nullptr, static_cast<uint32_t>(memoryBarriers.size()), memoryBarriers.data(),
            0, nullptr);
    }

    void CreateBufferView(VkDevice device,
        VkBuffer buffer,
        VkFormat format,
        VkDeviceSize memoryOffset,
        VkDeviceSize memoryRange,
        VkBufferView& bufferView)
    {
        VkBufferViewCreateInfo bufferInfo =
        {
            VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            nullptr,
            0,
            buffer,
            format,
            memoryOffset,
            memoryRange
        };

        VK_CHECK_RESULT(vkCreateBufferView(device, &bufferInfo, nullptr, &bufferView));
    }

    void CreateImage(VkDevice device,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t mipmapsCount,
        uint32_t layersCount,
        VkSampleCountFlagBits samples,
        VkImageUsageFlags usage,
        bool cubemap,
        VkImage& image)
    {
        VkImageCreateInfo imageInfo =
        {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            nullptr,
            cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
            type,
            format,
            size,
            mipmapsCount,
            cubemap ? 6 * layersCount : layersCount,
            samples,
            VK_IMAGE_TILING_OPTIMAL,
            usage,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr,
            VK_IMAGE_LAYOUT_UNDEFINED
        };

        VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &image));
    }

    void AllocateAndBindMemoryObjectToImage(VkDevice device,
        VkPhysicalDevice gpu,
        VkImage image,
        VkMemoryPropertyFlagBits memoryProperties,
        VkDeviceMemory& memoryObject)
    {
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(gpu, &physicalDeviceMemoryProperties);

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, image, &memoryRequirements);

        for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
                (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties)
            {
                VkMemoryAllocateInfo allocInfo =
                {
                    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    nullptr,
                    memoryRequirements.size,
                    i
                };

                VkResult res = vkAllocateMemory(device, &allocInfo, nullptr, &memoryObject);
                if (res == VK_SUCCESS)
                {
                    break;
                }
            }
        }

        if (memoryObject == VK_NULL_HANDLE)
        {
            WARN_LOG("Failed to create memory object for image");
            return;
        }

        VK_CHECK_RESULT(vkBindImageMemory(device, image, memoryObject, 0));
    }

    void SetImageMemoryBarrier(VkCommandBuffer commandBuffer,
        VkPipelineStageFlags generatingStages,
        VkPipelineStageFlags consumingStages,
        const std::vector<ImageTransition> imageTransition)
    {
        std::vector<VkImageMemoryBarrier> barriers;
        for (auto& transition : imageTransition)
        {
            barriers.push_back(
            {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                nullptr,
                transition.srcAccessFlags,
                transition.dstAccessFlags,
                transition.srcLayout,
                transition.dstLayout,
                transition.srcQueueFamilyIndex,
                transition.dstQueueFamilyIndex,
                transition.image,
                {
                    transition.aspect,
                    0,
                    VK_REMAINING_MIP_LEVELS,
                    0,
                    VK_REMAINING_ARRAY_LAYERS
                }
            });

            if (barriers.size() == 0)
            {
                return;
            }

            vkCmdPipelineBarrier(commandBuffer, generatingStages, consumingStages, 0, 0, nullptr, 0, nullptr,
                static_cast<uint32_t>(barriers.size()), barriers.data());
        }
    }
    void CreateImageView(VkDevice device, 
        VkImage image, 
        VkImageViewType viewType, 
        VkFormat format, 
        VkImageAspectFlags flags, 
        VkImageView& imageview)
    {
        VkImageViewCreateInfo imageInfo =
        {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            image,
            viewType,
            format,
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY
            },
            {
                flags,
                0,
                VK_REMAINING_MIP_LEVELS,
                0,
                VK_REMAINING_ARRAY_LAYERS
            }
        };

        VK_CHECK_RESULT(vkCreateImageView(device, &imageInfo, nullptr, &imageview));
    }

    void Create2DImageAndView(VkDevice device, 
        VkPhysicalDevice gpu, 
        VkFormat format, 
        VkExtent2D size, 
        uint32_t mipmapCount, 
        uint32_t layersCount, 
        VkSampleCountFlagBits samples, 
        VkImageUsageFlags usage, 
        VkImageAspectFlags aspect, 
        VkImage& image, 
        VkDeviceMemory& imageMemory, 
        VkImageView& imageView)
    {
        CreateImage(device, VK_IMAGE_TYPE_2D, format, { size.width, size.height, 1 }, mipmapCount, layersCount, samples, usage, false, image);
        AllocateAndBindMemoryObjectToImage(device, gpu, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageMemory);
        CreateImageView(device, image, VK_IMAGE_VIEW_TYPE_2D, format, aspect, imageView);
    }

    void CreateLayered2DImageWithCubemapView(VkDevice device, 
        VkPhysicalDevice gpu, 
        uint32_t size, 
        uint32_t mipmaps, 
        VkImageUsageFlags usage, 
        VkImageAspectFlags aspect, 
        VkImage& image, 
        VkDeviceMemory& imageMemory, 
        VkImageView& imageView)
    {
        CreateImage(device, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { size, size, 1 }, mipmaps, 6,
            VK_SAMPLE_COUNT_1_BIT, usage, true, image);
        AllocateAndBindMemoryObjectToImage(device, gpu, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageMemory);
        CreateImageView(device, image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, aspect, imageView);
    }
    void MapUpdateAndUnmapHostVisibleMemory(VkDevice device, 
        VkDeviceMemory memoryObject, 
        VkDeviceSize offset, 
        VkDeviceSize size, 
        void* data, 
        bool unmap, 
        void** pointer)
    {
        void* localPointer;

        VK_CHECK_RESULT(vkMapMemory(device, memoryObject, offset, size, 0, &localPointer));

        std::memcpy(localPointer, data, static_cast<size_t>(size));

        std::vector<VkMappedMemoryRange> ranges =
        {
            {
                VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                nullptr,
                memoryObject,
                offset,
                VK_WHOLE_SIZE
            }
        };

        vkFlushMappedMemoryRanges(device, 1, ranges.data());

        if (unmap)
        {
            vkUnmapMemory(device, memoryObject);
        }
        else
        {
            *pointer = localPointer;
        }
    }

    void CopyDataBetweenBuffers(VkCommandBuffer commandBuffer, 
        VkBuffer srcBuffer, 
        VkBuffer dstBuffer, 
        std::vector<VkBufferCopy> regions)
    {
        if (regions.size() == 0)
        {
            return;
        }

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, static_cast<uint32_t>(regions.size()), regions.data());
    }

    void CopyDataFromBufferToImage(VkCommandBuffer commandBuffer, 
        VkBuffer srcBuffer, 
        VkImage dstImage, 
        VkImageLayout imageLayout, 
        std::vector<VkBufferImageCopy> regions)
    {
        if (regions.size() == 0)
        {
            return;
        }

        vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, imageLayout, static_cast<uint32_t>(regions.size()),
            regions.data());
    }

    void CopyDataFromImageToBuffer(VkCommandBuffer commandBuffer, 
        VkImage srcImage, 
        VkImageLayout imageLayout, 
        VkBuffer dstBuffer, 
        std::vector<VkBufferImageCopy> regions)
    {
        if (regions.size() == 0)
        {
            return;
        }

        vkCmdCopyImageToBuffer(commandBuffer, srcImage, imageLayout, dstBuffer, static_cast<uint32_t>(regions.size()),
            regions.data());
    }

    bool UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(VkDevice device, 
        VkPhysicalDevice gpu, 
        VkDeviceSize dataSize, 
        void* data, 
        VkBuffer dstBuffer, 
        VkDeviceSize dstOffset, 
        VkAccessFlags dstCurrentAccess, 
        VkAccessFlags dstNewAcecess, 
        VkPipelineStageFlags dstGeneratingStage, 
        VkPipelineStageFlags dstConsumingStage, 
        VkQueue queue, 
        VkCommandBuffer commandBuffer, 
        std::vector<VkSemaphore> signalSemaphores)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
        AllocateAndBindMemoryObjectToBuffer(device, gpu, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBufferMemory);

        MapUpdateAndUnmapHostVisibleMemory(device, stagingBufferMemory, 0, dataSize, data, true, nullptr);

        BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);

        BufferTransition transition;
        transition.buffer = dstBuffer;
        transition.srcFlags = dstCurrentAccess;
        transition.dstFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
        transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        SetBufferMemoryBarrier(commandBuffer, dstGeneratingStage, VK_PIPELINE_STAGE_TRANSFER_BIT, { transition });

        VkBufferCopy copy;
        copy.dstOffset = dstOffset;
        copy.srcOffset = 0;
        copy.size = dataSize;
        CopyDataBetweenBuffers(commandBuffer, stagingBuffer, dstBuffer, { copy });

        transition.buffer = dstBuffer;
        transition.srcFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
        transition.dstFlags = dstNewAcecess;
        transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        SetBufferMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, dstConsumingStage, { transition });

        EndCommandBufferRecordingOperation(commandBuffer);

        VkFence fence;
        CreateVkFence(device, false, fence);

        SubmitCommandBuffersToQueue(queue, {}, { commandBuffer }, signalSemaphores, fence);

        std::vector<VkFence> fences = { fence };
        if (!WaitForFences(device, fences, VK_FALSE, 500000000))
        {
            return false;
        }

        vkFreeMemory(device, stagingBufferMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);

        return true;
    }

    bool UseStaginBufferToUpdateImageWithDeviceLocalMemoryBound(VkDevice device, 
        VkPhysicalDevice gpu, 
        VkDeviceSize dataSize, 
        void* data, 
        VkImage dstImage, 
        VkImageSubresourceLayers dstImageSubresources, 
        VkOffset3D dstImageOffset, 
        VkExtent3D dstImageSize, 
        VkImageLayout dstCurrentLayout, 
        VkImageLayout dstNewLayout, 
        VkAccessFlags dstCurrentAccess, 
        VkAccessFlags dstNewAccess, 
        VkImageAspectFlags aspect, 
        VkPipelineStageFlags dstImageGeneratingStage, 
        VkPipelineStageFlags dstImageConsumingStage, 
        VkQueue queue, 
        VkCommandBuffer commandBuffer, 
        std::vector<VkSemaphore> signalSemaphores)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
        AllocateAndBindMemoryObjectToBuffer(device, gpu, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBufferMemory);

        MapUpdateAndUnmapHostVisibleMemory(device, stagingBufferMemory, 0, dataSize, data, true, nullptr);

        BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);

        ImageTransition transition;
        transition.image = dstImage;
        transition.aspect = aspect;
        transition.srcAccessFlags = dstCurrentAccess;
        transition.dstAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
        transition.srcLayout = dstCurrentLayout;
        transition.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        SetImageMemoryBarrier(commandBuffer, dstImageGeneratingStage, VK_PIPELINE_STAGE_TRANSFER_BIT, { transition });

        VkBufferImageCopy copy;
        copy.bufferImageHeight = 0;
        copy.bufferRowLength = 0;
        copy.bufferOffset = 0;
        copy.imageExtent = dstImageSize;
        copy.imageOffset = dstImageOffset;
        copy.imageSubresource = dstImageSubresources;
        CopyDataFromBufferToImage(commandBuffer, stagingBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { copy });

        transition.image = dstImage;
        transition.aspect = aspect;
        transition.srcAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
        transition.dstAccessFlags = dstNewAccess;
        transition.srcLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transition.dstLayout = dstNewLayout;
        transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        SetImageMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, dstImageConsumingStage, { transition });

        EndCommandBufferRecordingOperation(commandBuffer);

        VkFence fence;
        CreateVkFence(device, false, fence);

        SubmitCommandBuffersToQueue(queue, {}, { commandBuffer }, signalSemaphores, fence);

        std::vector<VkFence> fences = { fence };
        if (!WaitForFences(device, fences, VK_FALSE, 500000000))
        {
            return false;
        }

        vkFreeMemory(device, stagingBufferMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);

        return true;
    }

    void DestroyImageView(VkDevice device, 
        VkImageView& view)
    {
        vkDestroyImageView(device, view, nullptr);
    }

    void DestroyImage(VkDevice device, 
        VkImage& image)
    {
        vkDestroyImage(device, image, nullptr);
    }

    void DestroyBufferView(VkDevice device, 
        VkBufferView& view)
    {
        vkDestroyBufferView(device, view, nullptr);
    }

    void DestroyBuffer(VkDevice device, 
        VkBuffer& buffer)
    {
        vkDestroyBuffer(device, buffer, nullptr);
    }

    void FreeDeviceMemoryObject(VkDevice device, 
        VkDeviceMemory& deviceMemory)
    {
        vkFreeMemory(device, deviceMemory, nullptr);
    }
}