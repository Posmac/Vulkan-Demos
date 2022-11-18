#pragma once

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

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

    struct UniformBufferObject
    {
        glm::mat4 ModelMatrix;
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
    };
}