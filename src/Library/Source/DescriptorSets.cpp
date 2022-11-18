#include "DescriptorSets.h"

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
        VkSampler& sampler)
    {
        VkSamplerCreateInfo samplerInfo =
        {
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            nullptr,
            0,
            magFilter,
            minFilter,
            mipmapMode,
            uAddressMode,
            vAddressMode,
            wAddressMode,
            lodBias,
            isAnisoEnabled,
            maxAnisotropy,
            compareOpEnabled,
            compareOp, 
            minLod, 
            maxLod,
            borderColor,
            unnormalizedCoords
        };

        VK_CHECK_RESULT(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
    }

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
        VkDeviceMemory& imageMemory)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        {
            WARN_LOG("Provided format is not supported for a sampled image");
            return false;
        }

        if (linearFiltering &&
            !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            WARN_LOG("Provided format is not supported for a linear image filtering");
            return false;
        }

        CreateImage(device, type, format, size, mipmaps, layers, VK_SAMPLE_COUNT_1_BIT, aspect, cubeMap, image);
        AllocateAndBindMemoryObjectToImage(device, gpu, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageMemory);
        CreateImageView(device, image, viewType, format, aspect, imageView);

        return true;
    }

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
        VkDeviceMemory& imageMemory)
    {
        CreateSampler(device, magFilter, minFilter, mipmapMode, uAddressMode, vAddressMode, wAddressMode,
            lodBias, isAnisoEnabled, maxAnisotropy, compareOpEnabled, compareOp, minLod, maxLod, 
            borderColor, unnormalizedCoords, sampler);

        bool linearFiltering = (magFilter == VK_FILTER_LINEAR) || (minFilter == VK_FILTER_LINEAR) ||
            (mipmapMode == VK_SAMPLER_MIPMAP_MODE_LINEAR);

        if (!CreateSampledImage(device, gpu, type, format, size, mipmaps, layers, usage, cubeMap, viewType, aspect,
            linearFiltering, image, imageView, imageMemory))
        {
            return false;
        }

        return true;
    }

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
        VkDeviceMemory& imageMemory)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
        {
            WARN_LOG("Provided format is not supported for storage image");
            return false;
        }

        if (atomicOperations && !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT))
        {
            WARN_LOG("Provided format is not supported for atomic operations on storage image");
            return false;
        }

        CreateImage(device, type, format, size, mipmaps, layers, VK_SAMPLE_COUNT_1_BIT, usage | VK_IMAGE_USAGE_STORAGE_BIT,
            false, image);
        AllocateAndBindMemoryObjectToImage(device, gpu, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageMemory);
        CreateImageView(device, image, viewType, format, aspect, imageView);

        return true;
    }

    bool CreateUniformTexelBuffer(VkDevice device, 
        VkPhysicalDevice gpu, 
        VkFormat format, 
        VkDeviceSize size, 
        VkImageUsageFlags usage, 
        VkBuffer& buffer, 
        VkBufferView& bufferView, 
        VkDeviceMemory& bufferMemory)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
        {
            WARN_LOG("Provided format is not supported for uniform texel buffer");
            return false;
        }

        CreateBuffer(device, size, usage, buffer);
        AllocateAndBindMemoryObjectToBuffer(device, gpu, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferMemory);
        CreateBufferView(device, buffer, format, 0, VK_WHOLE_SIZE, bufferView);

        return true;
    }

    bool CreateStorageTexelBuffer(VkDevice device, 
        VkPhysicalDevice gpu, 
        VkFormat format, 
        VkDeviceSize size, 
        VkBufferUsageFlags usage, 
        bool atomicOperations, 
        VkBuffer& buffer, 
        VkBufferView& bufferView, 
        VkDeviceMemory& bufferMemory)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
        {
            WARN_LOG("Provided format is not supported for storage texel buffer");
            return false;
        }

        if (atomicOperations && !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT))
        {
            WARN_LOG("Provided format is not supported for atomic operations on storage texel buffers");
            return false;
        }

        CreateBuffer(device, size, usage | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, buffer);
        AllocateAndBindMemoryObjectToBuffer(device, gpu, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferMemory);
        CreateBufferView(device, buffer, format, 0, VK_WHOLE_SIZE, bufferView);

        return true;
    }

    void CreateUniformBuffer(VkDevice device, 
        VkPhysicalDevice gpu, 
        VkDeviceSize size, 
        VkBufferUsageFlags usage, 
        VkBuffer& buffer, 
        VkDeviceMemory& bufferMemory)
    {
        CreateBuffer(device, size, usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, buffer);
        AllocateAndBindMemoryObjectToBuffer(device, gpu, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferMemory);
    }

    void CreateStorageBuffer(VkDevice device,
        VkPhysicalDevice gpu,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory)
    {
        CreateBuffer(device, size, usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, buffer);
        AllocateAndBindMemoryObjectToBuffer(device, gpu, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferMemory);
    }

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
        VkDeviceMemory& imageMemory)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProperties);
        if ((aspect & VK_IMAGE_ASPECT_COLOR_BIT) &&
            !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
        {
            WARN_LOG("Provided format is not supported for an input attachment");
            return false;
        }

        if((aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) &&
            !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
            WARN_LOG("Provided format is not supported for an input attachment");
            return false;
        }

        CreateImage(device, type, format, size, 1, 1, VK_SAMPLE_COUNT_1_BIT,
            usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, false, image);
        AllocateAndBindMemoryObjectToImage(device, gpu, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            imageMemory);
        CreateImageView(device, image, viewType, format, aspect, imageView);
        
        return true;
    }

    void CreateDescriptorSetLayout(VkDevice device, 
        const std::vector<VkDescriptorSetLayoutBinding>& bindings, 
        VkDescriptorSetLayout& descriptorSetLayout)
    {
        VkDescriptorSetLayoutCreateInfo descriptorInfo =
        {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(bindings.size()),
            bindings.data()
        };

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorInfo, nullptr, &descriptorSetLayout));
    }

    void CreateDescriptorPool(VkDevice device, 
        bool freeIndividualSets, 
        uint32_t maxSetsCount, 
        const std::vector<VkDescriptorPoolSize>& types, 
        VkDescriptorPool& descriptorPool)
    {
        VkDescriptorPoolCreateInfo poolInfo =
        {
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            nullptr,
            freeIndividualSets ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0u,
            maxSetsCount,
            static_cast<uint32_t>(types.size()),
            types.data()
        };

        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
    }

    void AllocateDescriptorSets(VkDevice device, 
        VkDescriptorPool descriptorPool, 
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, 
        std::vector<VkDescriptorSet>& descriptorSets)
    {
        VkDescriptorSetAllocateInfo allocInfo =
        {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            nullptr,
            descriptorPool,
            static_cast<uint32_t>(descriptorSetLayouts.size()),
            descriptorSetLayouts.data()
        };

        descriptorSets.resize(descriptorSetLayouts.size());

        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()));
    }

    void UpdateDescriptorSets(VkDevice device, 
        const std::vector<ImageDescriptorInfo>& imageInfo,
        const std::vector<BufferDescriptorInfo>& bufferInfo,
        const std::vector<TexelBufferDescriptorInfo>& texelInfo,
        const std::vector<CopyDescriptorInfo>& copyInfo)
    {
        std::vector<VkWriteDescriptorSet> writeDescriptors;
        std::vector<VkCopyDescriptorSet> copyDescriptors;

        for (auto& image : imageInfo)
        {
            writeDescriptors.push_back(
                {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    image.descriptorSet,
                    image.binding,
                    image.arrayElement,
                    static_cast<uint32_t>(image.imageInfo.size()),
                    image.descriptorType,
                    image.imageInfo.data(),
                    nullptr,
                    nullptr
                }
            );
        }

        for (auto& buffer : bufferInfo)
        {
            writeDescriptors.push_back(
                {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    buffer.descriptorSet,
                    buffer.binding,
                    buffer.arrayElement,
                    static_cast<uint32_t>(buffer.bufferInfo.size()),
                    buffer.descriptorType,
                    nullptr,
                    buffer.bufferInfo.data(),
                    nullptr
                }
            );
        }

        for (auto& texel : texelInfo)
        {
            writeDescriptors.push_back(
                {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,
                    texel.descriptorSet,
                    texel.binding,
                    texel.arrayElement,
                    static_cast<uint32_t>(texel.bufferInfo.size()),
                    texel.descriptorType,
                    nullptr,
                    nullptr,
                    texel.bufferInfo.data(),
                }
            );
        }

        for (auto& copy : copyDescriptors)
        {
            copyDescriptors.push_back(
                {
                    VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
                    nullptr,
                    copy.srcSet,
                    copy.srcBinding,
                    copy.srcArrayElement,
                    copy.dstSet,
                    copy.dstBinding,
                    copy.dstArrayElement,
                    copy.descriptorCount
                }
            );
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(),
            static_cast<uint32_t>(copyDescriptors.size()), copyDescriptors.data());
    }

    void BindDescitorSets(VkCommandBuffer commandBuffer, 
        VkPipelineBindPoint pipelineType, 
        VkPipelineLayout layout, 
        uint32_t indexForFirstSet, 
        const std::vector<VkDescriptorSet>& sets, 
        const std::vector<uint32_t>& dynamicOffsets)
    {
        vkCmdBindDescriptorSets(commandBuffer, pipelineType, layout, indexForFirstSet,
            static_cast<uint32_t>(sets.size()), sets.data(),
            static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
    }

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
        std::vector<VkDescriptorSet>& descriptorSets)
    {
        CreateCombinedImageSampler(device, gpu, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, sampledImageSize, 1, 1,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,
            VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
            0.0f, false, 1.0f, false, VK_COMPARE_OP_ALWAYS, 0.0f, 0.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
            false, sampler, sampledImage, sampledImageView, sampledImageMemory);

        CreateUniformBuffer(device, gpu, uniformBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, uniformBuffer, uniformBufferMemory);

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

        CreateDescriptorSetLayout(device, bindings, descriptorSetLayout);

        std::vector<VkDescriptorPoolSize> types =
        {
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1
            },
            {
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1
            }
        };

        CreateDescriptorPool(device, false, 1, types, descriptorPool);


        AllocateDescriptorSets(device, descriptorPool, { descriptorSetLayout }, descriptorSets);

        std::vector<ImageDescriptorInfo> imageDescriptors =
        {
            {
                descriptorSets[0], 
                0, 
                0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                {
                    {
                        sampler,
                        sampledImageView,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    }
                }
            },
        };

        std::vector<BufferDescriptorInfo> bufferDescriptors =
        {
            {
                descriptorSets[0],
                1,
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                {
                    {
                        uniformBuffer,
                        0,
                        VK_WHOLE_SIZE,
                    }
                }
            },
        };

        UpdateDescriptorSets(device, imageDescriptors, bufferDescriptors, {}, {});
    }

    void FreeDescriptorSets(VkDevice device, 
        VkDescriptorPool descriptorPool, 
        std::vector<VkDescriptorSet>& sets)
    {
        VK_CHECK_RESULT(vkFreeDescriptorSets(device, descriptorPool, static_cast<uint32_t>(sets.size()), sets.data()));
    }

    void ResetDescriptorPool(VkDevice device, 
        VkDescriptorPool pool)
    {
        VK_CHECK_RESULT(vkResetDescriptorPool(device, pool, 0));
    }

    void DestroyDescriptorPool(VkDevice device, 
        VkDescriptorPool& pool)
    {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }

    void DestroyDescriptorSetLayout(VkDevice device, 
        VkDescriptorSetLayout& layout)
    {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
    }
      
    void DestroySampler(VkDevice device, 
        VkSampler& sampler)
    {
        vkDestroySampler(device, sampler, nullptr);
    }
}