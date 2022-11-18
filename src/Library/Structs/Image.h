#pragma once

#include "vulkan/vulkan.h"

namespace vk
{
    struct Image
    {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
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

    struct SwapchainParameters
    {
        VkSwapchainKHR handle;
        VkFormat format;
        VkExtent2D size;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
    };

    struct PresentInfo
    {
        VkSwapchainKHR  swapchain;
        uint32_t        imageIndex;
    };
}