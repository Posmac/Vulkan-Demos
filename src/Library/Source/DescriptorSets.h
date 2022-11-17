#pragma once

#include "Library/Core/Core.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandBuffer.h"
#include "Resources.h"
#include "Library/Structs/Buffer.h"
#include "Library/Structs/Image.h"
#include "Library/Structs/Descriptors.h"

namespace vk
{
    void CreateSampler(VkDevice device,
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerMipmapMode mipmapMode,
        VkSamplerAddressMode uAddressMode,
        VkSamplerAddressMode vAddressMode,
        VkSamplerAddressMode wAddressMode,
        float lodBias,
        bool isAnisoEnabled,
        float maxAnisotropy,
        bool compareOpEnabled,
        VkCompareOp compareOp,
        float minLod,
        float maxLod,
        VkBorderColor borderColor,
        bool unnormalizedCoords,
        VkSampler& sampler);

    bool CreateSampledImage(VkDevice device,
        VkPhysicalDevice gpu,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t mipmaps,
        uint32_t layers,
        VkImageUsageFlags usage,
        bool cubeMap,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        bool linearFiltering,
        VkImage& image,
        VkImageView& imageView,
        VkDeviceMemory& imageMemory);

    bool CreateCombinedImageSampler(VkDevice device,
        VkPhysicalDevice gpu,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t mipmaps,
        uint32_t layers,
        VkImageUsageFlags usage,
        bool cubeMap,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerMipmapMode mipmapMode,
        VkSamplerAddressMode uAddressMode,
        VkSamplerAddressMode vAddressMode,
        VkSamplerAddressMode wAddressMode,
        float lodBias,
        bool isAnisoEnabled,
        float maxAnisotropy,
        bool compareOpEnabled,
        VkCompareOp compareOp,
        float minLod,
        float maxLod,
        VkBorderColor borderColor,
        bool unnormalizedCoords,
        VkSampler& sampler,
        VkImage& image,
        VkImageView& imageView,
        VkDeviceMemory& imageMemory);

    bool CreateStorageImage(VkDevice device,
        VkPhysicalDevice gpu,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        uint32_t mipmaps,
        uint32_t layers,
        VkImageUsageFlags usage,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        bool atomicOperations,
        VkImage& image,
        VkImageView& imageView,
        VkDeviceMemory& imageMemory);

    bool CreateUniformTexelBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkFormat format,
        VkDeviceSize size,
        VkImageUsageFlags usage,
        VkBuffer& buffer,
        VkBufferView& bufferView,
        VkDeviceMemory& bufferMemory);

    bool CreateStorageTexelBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkFormat format,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        bool atomicOperations,
        VkBuffer& buffer,
        VkBufferView& bufferView,
        VkDeviceMemory& bufferMemory);

    void CreateUniformBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    void CreateStorageBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    bool CreateInputAttachment(VkDevice device,
        VkPhysicalDevice gpu,
        VkImageType type,
        VkFormat format,
        VkExtent3D size,
        VkImageUsageFlags usage,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        VkImage& image,
        VkImageView& imageView,
        VkDeviceMemory& imageMemory);

    void CreateDescriptorSetLayout(VkDevice device,
        std::vector<VkDescriptorSetLayoutBinding>& bindings,
        VkDescriptorSetLayout& descriptorSetLayout);

    void CreateDescriptorPool(VkDevice device,
        bool freeIndividualSets,
        uint32_t maxSetsCount,
        std::vector<VkDescriptorPoolSize>& types,
        VkDescriptorPool& descriptorPool);

    void AllocateDescriptorSets(VkDevice device,
        VkDescriptorPool descriptorPool,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        std::vector<VkDescriptorSet>& descriptorSets);

    void UpdateDescriptorSets(VkDevice device,
        const std::vector<ImageDescriptorInfo>& imageInfo,
        const std::vector<BufferDescriptorInfo>& bufferInfo,
        const std::vector<TexelBufferDescriptorInfo>& texelInfo,
        const std::vector<CopyDescriptorInfo>& copyInfo);

    void BindDescitorSets(VkCommandBuffer commandBuffer,
        VkPipelineBindPoint pipelineType,
        VkPipelineLayout layout,
        uint32_t indexForFirstSet,
        const std::vector<VkDescriptorSet>& sets,
        const std::vector<uint32_t>& dynamicOffsets);

    void CreateDescriptorsWithTextreAndUniformBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkExtent3D sampledImageSize,
        uint32_t uniformBufferSize,
        VkSampler& sampler,
        VkImage& sampledImage,
        VkImageView& sampledImageView,
        VkDeviceMemory& sampledImageMemory,
        VkBuffer& uniformBuffer,
        VkDeviceMemory& uniformBufferMemory,
        VkDescriptorSetLayout& descriptorSetLayout,
        VkDescriptorPool& descriptorPool,
        std::vector<VkDescriptorSet>& descriptorSets);

    void FreeDescriptorSets(VkDevice device,
        VkDescriptorPool descriptorPool,
        std::vector<VkDescriptorSet>& sets);

    void ResetDescriptorPool(VkDevice device,
        VkDescriptorPool pool);

    void DestroyDescriptorPool(VkDevice device,
        VkDescriptorPool& pool);

    void DestroyDescriptorSetLayout(VkDevice device,
        VkDescriptorSetLayout& layout);

    void DestroySampler(VkDevice device,
        VkSampler& sampler);
}