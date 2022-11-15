#pragma once

#include "vulkan/vulkan.h"

namespace vk
{
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
}