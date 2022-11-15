#pragma once

#include <algorithm>

#include "Core.h"
#include "QueueInfo.h"
#include "WindowParams.h"
#include <vulkan/vulkan_win32.h>

namespace vk
{
    class Swapchain
    {
    public:
        Swapchain();
        void Destroy(const VkDevice& device, const VkInstance& instance);
        void Init(const VkInstance& instance, const VkPhysicalDevice& physicalDevice,
            const QueuesInfo& info, const WindowParameters& params);
        void PresentImage(const VkDevice& device, const VkQueue& graphicsQueue);
        void CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice,
            VkImageUsageFlags usageFlags, VkSurfaceTransformFlagBitsKHR surfaceTransform,
            VkSurfaceFormatKHR desiredFormat, int width, int height);
        std::vector<VkImage> GetImages();

        VkSurfaceKHR GetSurface() const;
        VkExtent2D GetExtent() const;
        VkSwapchainKHR GetSwapchain() const;
    private:
        void CreateSurface(const VkInstance& instance, const VkPhysicalDevice& physicalDevice,
            const QueuesInfo& info, const WindowParameters& params);
        void SelectPresentationMode(const VkPresentModeKHR desiredPresentMode, const VkPhysicalDevice& physicalDevice);
        void GetPresentationSurfaceCapabilities(const VkPhysicalDevice& physicalDevice);


    private:
        int width;
        int height;

        VkSurfaceKHR surface;
        VkPresentModeKHR presentMode;
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        VkExtent2D surfaceExtent{};

        VkSurfaceFormatKHR surfaceFormat;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;

        uint32_t currentImageIndex;
        VkSemaphore semaphore;
        VkFence fence;
        std::vector<VkSemaphore> swapchainSemaphores;
    };
}