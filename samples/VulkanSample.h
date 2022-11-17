#pragma once

#include "VulkanSampleBase.h"

namespace vk
{
    class Sample : public SampleBase
    {
    public:
        bool InitVulkan(WindowParameters& windowParams,
            std::vector<const char*> validationLayer,
            std::vector<const char*> instanceExtensions,
            std::vector<const char*> deviceExtensions,
            VkPhysicalDeviceFeatures* deviceFeatures = nullptr,
            bool useDepth = true,
            VkImageUsageFlags swapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        bool CreateSwapchain(bool useDepth = true,
            VkImageUsageFlags swapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        virtual bool Initialize(WindowParameters& windowParams,
            std::vector<const char*> validationLayer,
            std::vector<const char*> instanceExtensions,
            std::vector<const char*> deviceExtensions) override;
        virtual bool Draw()  override;
        virtual bool Resize()  override;
        virtual void Destroy()  override;

    private:
        VkDebugUtilsMessengerEXT messenger;
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR presentationSurface;
        QueueParameters graphicsQueue;
        QueueParameters presentQueue;
        QueueParameters computeQueue;
        SwapchainParameters swapchain;
        VkCommandPool commandPool;
        std::vector<VkImage> depthImages;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkDeviceMemory> depthImageMemory;
        std::vector<FrameResources> frameResources;
        uint32_t framesCount = 3;
        VkFormat depthFormat = VK_FORMAT_D16_UNORM;
    };
}