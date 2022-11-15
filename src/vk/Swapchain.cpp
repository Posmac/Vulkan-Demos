#include "Swapchain.h"

namespace vk
{
    Swapchain::Swapchain()
    {

    }

    void Swapchain::Init(const VkInstance& instance, const VkPhysicalDevice& physicalDevice,
        const QueuesInfo& info, const WindowParameters& params)
    {
        CreateSurface(instance, physicalDevice, info, params);
        SelectPresentationMode(VK_PRESENT_MODE_MAILBOX_KHR, physicalDevice);
        GetPresentationSurfaceCapabilities(physicalDevice);
    }

    void Swapchain::PresentImage(const VkDevice& device, const VkQueue& graphicsQueue)
    {
        auto result = vkAcquireNextImageKHR(device, swapchain, 2000000000, 0, 0, &currentImageIndex);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.pResults = nullptr;

        VkSemaphore semaphores[] = { swapchainSemaphores[currentImageIndex] };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = semaphores;

        VkSwapchainKHR swapChains[] = { swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &currentImageIndex;

        vkQueuePresentKHR(graphicsQueue, &presentInfo);
    }

    void Swapchain::Destroy(const VkDevice& device, const VkInstance& instance)
    {
        if (swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, swapchain, nullptr);
        }

        if (surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }
    }

    void Swapchain::CreateSurface(const VkInstance& instance, const VkPhysicalDevice& physicalDevice,
        const QueuesInfo& info, const WindowParameters& params)
    {
        VkWin32SurfaceCreateInfoKHR surfaceInfo{};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceInfo.pNext = nullptr;
        surfaceInfo.hinstance = params.hInstance;
        surfaceInfo.hwnd = params.hWnd;
        surfaceInfo.flags = 0;

        VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);

        if (result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create Win32 surface");
        }

        //checking if physical device support presentaions to a given surface
        VkBool32 isPresantationSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, info.graphicsFamily.value(), surface, &isPresantationSupported);

        if (!isPresantationSupported)
        {
            LOG_INFO("Presentation isnt supported by current physical device");
        }
    }

    void Swapchain::SelectPresentationMode(const VkPresentModeKHR desiredPresentMode, const VkPhysicalDevice& physicalDevice)
    {
        uint32_t presentModesCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, presentModes.data());

        for (const auto pMode : presentModes)
        {
            if (pMode == desiredPresentMode)
            {
                presentMode = desiredPresentMode;
                return;
            }
        }

        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    void Swapchain::GetPresentationSurfaceCapabilities(const VkPhysicalDevice& physicalDevice)
    {
        auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
        if (result != VK_SUCCESS)
        {
            LOG_INFO("cannot obtain presentation surface capabilities");
        }
    }

    VkSurfaceKHR Swapchain::GetSurface() const
    {
        return surface;
    }

    VkExtent2D Swapchain::GetExtent() const
    {
        return surfaceExtent;
    }

    VkSwapchainKHR Swapchain::GetSwapchain() const
    {
        return swapchain;
    }

    void Swapchain::CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice,
        VkImageUsageFlags usageFlags, VkSurfaceTransformFlagBitsKHR surfaceTransform,
        VkSurfaceFormatKHR desiredFormat, int width, int height)
    {
        uint32_t imagesNumber = surfaceCapabilities.minImageCount + 1;
        imagesNumber = imagesNumber < surfaceCapabilities.maxImageCount ? imagesNumber : surfaceCapabilities.maxImageCount;

        if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
        {
            VkExtent2D surfaceExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            surfaceExtent.width = std::clamp(surfaceExtent.width,
                surfaceCapabilities.minImageExtent.width,
                surfaceCapabilities.maxImageExtent.width);
            surfaceExtent.height = std::clamp(surfaceExtent.height,
                surfaceCapabilities.minImageExtent.height,
                surfaceCapabilities.maxImageExtent.height);
        }
        else
        {
            surfaceExtent = surfaceCapabilities.currentExtent;
        }

        uint32_t formatsCout = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCout, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatsCout);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCout, formats.data());

        if (formats[0].format == VK_FORMAT_UNDEFINED)
        {
            surfaceFormat = desiredFormat;
        }

        VkSwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.pNext = nullptr;
        swapchainInfo.flags = 0;
        swapchainInfo.surface = surface;
        swapchainInfo.minImageCount = imagesNumber;
        swapchainInfo.imageFormat = desiredFormat.format;
        swapchainInfo.imageColorSpace = desiredFormat.colorSpace;
        swapchainInfo.imageExtent = surfaceExtent;
        swapchainInfo.imageUsage = usageFlags;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
        swapchainInfo.preTransform = surfaceTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = presentMode;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.oldSwapchain = nullptr;

        VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));

        uint32_t swapchainImagesCount;
        vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, nullptr);
        swapchainImages.resize(swapchainImagesCount);
        vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, swapchainImages.data());
    }

    std::vector<VkImage> Swapchain::GetImages()
    {
        return swapchainImages;
    }
}