#pragma once

#include <algorithm>

#include "Core.h"
#include "QueueInfo.h"
#include "Win32Window.h"

namespace vk
{
    class CVkSwapchain
    {
    public:
        CVkSwapchain();
        void Destroy(const VkDevice& device, const VkInstance& instance);
        void Init(const VkInstance& instance, const VkPhysicalDevice& physicalDevice,
                        const QueuesInfo& info, const WindowParameters& params);
        void PresentImage(const VkDevice& device, const VkQueue& graphicsQueue);
        void CreateSwapchain(VkImageUsageFlags usageFlags, VkSurfaceTransformFlagBitsKHR surfaceTransform,
                            VkSurfaceFormatKHR desiredFormat, int width, int height, 
                            const VkDevice& device, const VkPhysicalDevice& physicalDevice);
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
        VkExtent2D surfaceExtent {};

		VkSurfaceFormatKHR surfaceFormat;
		VkSwapchainKHR swapchain;
		std::vector<VkImage> swapchainImages;

		uint32_t currentImageIndex;
		VkSemaphore semaphore;
		VkFence fence;
		std::vector<VkSemaphore> swapchainSemaphores;
    };
}