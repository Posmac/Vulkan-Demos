#pragma once

#include "Library/Core/Core.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandBuffer.h"
#include "Library/Structs/Buffer.h"
#include "Library/Structs/Image.h"

namespace vk
{
    void CreateBuffer(VkDevice device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& buffer);

    void AllocateAndBindMemoryObjectToBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkBuffer buffer,
        VkMemoryPropertyFlagBits memoryProperties,
        VkDeviceMemory& memoryObject);

    void SetBufferMemoryBarrier(VkCommandBuffer commandBuffer,
        VkPipelineStageFlags generatingStages,
        VkPipelineStageFlags consumingStages,
        std::vector<BufferTransition> bufferTransitions);

    void CreateBufferView(VkDevice device,
        VkBuffer buffer,
        VkFormat format,
        VkDeviceSize memoryOffset,
        VkDeviceSize memoryRange,
        VkBufferView& bufferView);

    void CreateImage(VkDevice device,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t mipmapsCount,
        uint32_t layersCount,
        VkSampleCountFlagBits samples,
        VkImageUsageFlags usage,
        bool cubemap,
        VkImage& image);

    void AllocateAndBindMemoryObjectToImage(VkDevice device,
        VkPhysicalDevice gpu,
        VkImage image,
        VkMemoryPropertyFlagBits memoryProperties,
        VkDeviceMemory& memoryObject);

    void SetImageMemoryBarrier(VkCommandBuffer commandBuffer,
        VkPipelineStageFlags generatingStages,
        VkPipelineStageFlags consumingStages,
        const std::vector<ImageTransition> imageTransition);

    void CreateImageView(VkDevice device,
        VkImage image,
        VkImageViewType viewType,
        VkFormat format,
        VkImageAspectFlags flags,
        VkImageView& imageview);

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
        VkImageView& imageView);

    void CreateLayered2DImageWithCubemapView(VkDevice device,
        VkPhysicalDevice gpu, 
        uint32_t size,
        uint32_t mipmaps, 
        VkImageUsageFlags usage,
        VkImageAspectFlags aspect,
        VkImage& image,
        VkDeviceMemory& imageMemory,
        VkImageView &imageView);

    void MapUpdateAndUnmapHostVisibleMemory(VkDevice device,
        VkDeviceMemory memoryObject,
        VkDeviceSize offset,
        VkDeviceSize size,
        void* data,
        bool unmap,
        void** pointer);

    void CopyDataBetweenBuffers(VkCommandBuffer commandBuffer,
        VkBuffer srcBuffer,
        VkBuffer dstBuffer,
        std::vector<VkBufferCopy> regions);

    void CopyDataFromBufferToImage(VkCommandBuffer commandBuffer,
        VkBuffer srcBuffer,
        VkImage dstImage,
        VkImageLayout imageLayout,
        std::vector<VkBufferImageCopy> regions);

    void CopyDataFromImageToBuffer(VkCommandBuffer commandBuffer,
        VkImage srcImage,
        VkImageLayout imageLayout,
        VkBuffer dstBuffer,
        std::vector<VkBufferImageCopy> regions);

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
        std::vector<VkSemaphore> signalSemaphores);

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
        std::vector<VkSemaphore> signalSemaphores);

    void DestroyImageView(VkDevice device,
        VkImageView& view);

    void DestroyImage(VkDevice device, 
        VkImage& image);

    void DestroyBufferView(VkDevice device,
        VkBufferView& view);

    void DestroyBuffer(VkDevice device,
        VkBuffer& image);

    void FreeDeviceMemoryObject(VkDevice device, 
        VkDeviceMemory& deviceMemory);
}