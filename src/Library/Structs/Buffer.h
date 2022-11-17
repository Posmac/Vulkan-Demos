#pragma once

#include "vulkan/vulkan.h"

namespace vk
{
    struct Buffer
    {
        VkBuffer buffer;
        VkBufferView bufferView;
    };

    struct VertexBufferParams
    {
        VkBuffer buffer;
        VkDeviceSize memoryOffset;
    };

    struct BufferDescriptorInfo
    {
        VkDescriptorSet descriptorSet;
        uint32_t binding;
        uint32_t arrayElement;
        VkDescriptorType descriptorType;
        std::vector<VkDescriptorBufferInfo> bufferInfo;
    };

    struct BufferTransition
    {
        VkBuffer buffer;
        VkAccessFlags srcFlags;
        VkAccessFlags dstFlags;
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;
    };
}