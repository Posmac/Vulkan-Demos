#pragma once

#include "vulkan/vulkan.h"

namespace vk
{
    struct Image
    {
        VkImage image;
        VkImageView view;
    };

    struct ImageDescriptorInfo
    {
        VkDescriptorSet descriptorSet;
        uint32_t binding;
        uint32_t arrayElement;
        VkDescriptorType descriptorType;
        std::vector<VkDescriptorImageInfo> imageInfo;
    };

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

    struct PresentInfo 
    {
        VkSwapchainKHR  Swapchain;
        uint32_t        ImageIndex;
    };
}