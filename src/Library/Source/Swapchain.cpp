#include "Swapchain.h"

namespace vk
{
    bool CreatePresentationSurface(VkInstance instance, 
        WindowParameters windowParameters, 
        VkSurfaceKHR& presentationSurface)
    {
        VkWin32SurfaceCreateInfoKHR surfaceInfo = 
        {
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            nullptr,
            0, 
            windowParameters.hInstance,
            windowParameters.hWnd
        };

        VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &presentationSurface));

        if (presentationSurface == VK_NULL_HANDLE)
        {
            CRITICAL_LOG("Failed to create presentation surface");
            return false;
        }

        return true;
    }

    bool SelectQueueFamilyThatSupportsPresentationToGivenSurface(VkPhysicalDevice device, 
        VkSurfaceKHR presentationSurface, 
        uint32_t& queueFamilyIndex)
    {
        std::vector<VkQueueFamilyProperties> queueFamilies;
        if (!CheckAvailableQueueFamiliesAndTheirProperties(device, queueFamilies))
        {
            return false;
        }

        for (int i = 0; i < queueFamilies.size(); i++)
        {
            VkBool32 presentationSupported = VK_FALSE;
            VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, presentationSurface, &presentationSupported));
            if (presentationSupported)
            {
                queueFamilyIndex = static_cast<uint32_t>(i);
                return true;
            }
        }

        return false;
    }

    bool SelectDesiredPresentationMode(VkPhysicalDevice device, 
        VkSurfaceKHR presentationSurface, 
        VkPresentModeKHR desiredPresentMode, 
        VkPresentModeKHR& presentMode)
    {
        uint32_t presentModesCount = 0;
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, presentationSurface, &presentModesCount, nullptr));

        if (presentModesCount == 0)
        {
            WARN_LOG("Could not get the number of present modes supported");
            return false;
        }

        std::vector<VkPresentModeKHR> presentModes(presentModesCount);
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, presentationSurface, &presentModesCount, presentModes.data()));

        if (presentModes.size() == 0)
        {
            WARN_LOG("Could not enumerate present modes");
            return false;
        }

        for (auto& currentPresentMode : presentModes)
        {
            if (currentPresentMode == desiredPresentMode)
            {
                presentMode = currentPresentMode;
                return true;
            }
        }

        //WARN_LOG("Desired present mode is not supported, selecting FIFO mode");
        for (auto& currentPresentMode : presentModes)
        {
            if (currentPresentMode == VK_PRESENT_MODE_FIFO_KHR)
            {
                presentMode = currentPresentMode;
                return true;
            }
        }

        CRITICAL_LOG("VK_PRESENT_MODE_FIFO_KHR is not supported by your device");
        return false;
    }

    bool GetCapabilitiesOfPresentationSurface(VkPhysicalDevice device,
        VkSurfaceKHR presentationSurface,
        VkSurfaceCapabilitiesKHR& surfaceCapabilities)
    {
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, presentationSurface, &surfaceCapabilities));
        return true;
    }

    bool SelectingNumberOfSwapchainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32_t& imagesNumber)
    {
        imagesNumber = surfaceCapabilities.minImageCount + 1;

        if (surfaceCapabilities.maxImageCount > 0 &&
            imagesNumber > surfaceCapabilities.maxImageCount)
        {
            imagesNumber = surfaceCapabilities.maxImageCount;
        }

        return true;
    }

    bool ChooseSizeOfSwapchainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities, VkExtent2D desiredSize, VkExtent2D& size)
    {
        if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
        {
            size.width = std::clamp(desiredSize.width, 
                surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            size.height = std::clamp(desiredSize.height, 
                surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        }
        else
        {
            size = surfaceCapabilities.currentExtent;
        }

        return true;
    }

    bool SelectDesiredUsageScenariosOfSwapChainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities, 
        VkImageUsageFlags desiredUsage, 
        VkImageUsageFlags& imageUsage)
    {
        imageUsage = desiredUsage & surfaceCapabilities.supportedUsageFlags;
        return desiredUsage == imageUsage;
    }

    bool SelectTransformationOfSwapchainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities, 
        VkSurfaceTransformFlagBitsKHR desiredTransform, 
        VkSurfaceTransformFlagBitsKHR& surfaceTransform)
    {
        if (surfaceCapabilities.supportedTransforms & desiredTransform)
        {
            surfaceTransform = desiredTransform;
        }
        else
        {
            surfaceTransform = surfaceCapabilities.currentTransform;
        }

        return true;
    }

    bool SelectFormatOfSwapChainImage(VkPhysicalDevice device, 
        VkSurfaceKHR presentationSurface, 
        VkSurfaceFormatKHR desiredSurfaceFormat, 
        VkFormat& imageFormat, 
        VkColorSpaceKHR& imageColorSpace)
    {
        uint32_t formatsCount = 0;
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, presentationSurface, &formatsCount, nullptr));
        if (formatsCount == 0)
        {
            WARN_LOG("Could not get the number of supported formats");
            return false;
        }

        std::vector<VkSurfaceFormatKHR> availableFormats(formatsCount);
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, presentationSurface, &formatsCount, availableFormats.data()));

        if (availableFormats.size() == 0)
        {
            WARN_LOG("Could not get available formats");
            return false;
        }

        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            imageFormat = desiredSurfaceFormat.format;
            imageColorSpace = desiredSurfaceFormat.colorSpace;
            return true;
        }

        for (auto& formats : availableFormats)
        {
            if (desiredSurfaceFormat.format & formats.format &&
                desiredSurfaceFormat.colorSpace & formats.colorSpace)
            {
                imageFormat = formats.format;
                imageColorSpace = formats.colorSpace;
                return true;
            }
        }

        for (auto& formats : availableFormats)
        {
            if (desiredSurfaceFormat.format == formats.format)
            {
                imageFormat = desiredSurfaceFormat.format;
                imageColorSpace = formats.colorSpace;
                WARN_LOG("Desired combination of format and color space is not supported. Selected other colorspace");
                return true;
            }
        }

        WARN_LOG("Desired combination of format and color space is not supported. Selected other combination");
        imageFormat = availableFormats[0].format;
        imageColorSpace = availableFormats[0].colorSpace;

        return true;
    }

    bool CreateSwapchain(VkDevice device, 
        VkSurfaceKHR presentationSurface, 
        uint32_t imagesCount, 
        VkSurfaceFormatKHR surfaceFormat, 
        VkExtent2D imageSize, 
        VkImageUsageFlags imageUsage, 
        VkSurfaceTransformFlagBitsKHR surfaceTransform,
        VkPresentModeKHR presentMode, 
        VkSwapchainKHR& oldSwapchain, 
        VkSwapchainKHR& swapchain)
    {
        VkSwapchainCreateInfoKHR swapchainInfo =
        {
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            nullptr,
            0,
            presentationSurface,
            imagesCount,
            surfaceFormat.format,
            surfaceFormat.colorSpace,
            imageSize,
            1,
            imageUsage,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr,
            surfaceTransform,
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            presentMode,
            VK_TRUE,
            oldSwapchain
        };

        VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));

        if (swapchain == VK_NULL_HANDLE)
        {
            CRITICAL_LOG("Failed to create swapchain");
            return false;
        }

        return true;
    }

    bool GetHandlesOfSwapChainImages(VkDevice device, 
        VkSwapchainKHR swapchain, 
        std::vector<VkImage>& swapchainImages)
    {
        uint32_t swapchainImagesCount = 0;
        VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, nullptr));

        if (swapchainImagesCount == 0)
        {
            WARN_LOG("Could not get the number of swapcain images");
            return false;
        }

        swapchainImages.resize(swapchainImagesCount);
        VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImagesCount, swapchainImages.data()));

        if (swapchainImages.size() == 0)
        {
            WARN_LOG("Failed to get swapchain images");
            return false;
        }

        return true;
    }

    bool CreateSwapchainWithRGBA8FormatAndMailBoxPresentMode(VkDevice device, 
        VkPhysicalDevice gpu, 
        VkSurfaceKHR presentationSufrace, 
        VkImageUsageFlags imageUsage, 
        VkExtent2D& imageSize, 
        VkFormat& imageFormat, 
        VkSwapchainKHR& oldSwapchain, 
        VkSwapchainKHR& swapchain, 
        std::vector<VkImage>& swapchainImages)
    {
        VkPresentModeKHR desiredMode;
        if (!SelectDesiredPresentationMode(gpu, presentationSufrace, VK_PRESENT_MODE_MAILBOX_KHR, desiredMode))
        {
            return false;
        }

        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        if (!GetCapabilitiesOfPresentationSurface(gpu, presentationSufrace, surfaceCapabilities))
        {
            return false;
        }

        uint32_t numberOfImages;
        if (!SelectingNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
        {
            return false;
        }

        VkExtent2D desiredSize = imageSize;
        if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, imageSize, desiredSize))
        {
            return false;
        }
        imageSize = desiredSize;

        VkImageUsageFlags desiredImageUsage;
        if (!SelectDesiredUsageScenariosOfSwapChainImages(surfaceCapabilities, imageUsage, desiredImageUsage))
        {
            return false;
        }

        VkSurfaceTransformFlagBitsKHR desiredTransform;
        SelectTransformationOfSwapchainImages(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, desiredTransform);

        VkColorSpaceKHR desiredColorSpace;
        if (!SelectFormatOfSwapChainImage(gpu, presentationSufrace, { VK_FORMAT_R8G8B8A8_UNORM , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            imageFormat, desiredColorSpace))
        {
            return false;
        }

        if (!CreateSwapchain(device, presentationSufrace, numberOfImages, { imageFormat, desiredColorSpace },
            desiredSize, desiredImageUsage, desiredTransform, desiredMode, oldSwapchain, swapchain))
        {
            return false;
        }

        if (!GetHandlesOfSwapChainImages(device, swapchain, swapchainImages))
        {
            return false;
        }

        return true;
    }

    bool AcquireSwapchainImage(VkDevice device, 
        VkSwapchainKHR swapchain, 
        VkSemaphore semaphore, 
        VkFence fence, 
        uint32_t& imageIndex)
    {
        VK_CHECK_RESULT(vkAcquireNextImageKHR(device, swapchain, 2000000000, semaphore, fence, &imageIndex));
        return true;
    }

    bool PresentImage(VkQueue queue, 
        std::vector<VkSemaphore> renderingSemaphores, 
        std::vector<PresentInfo> imagesToPresent)
    {
        std::vector<VkSwapchainKHR> swapchains;
        std::vector<uint32_t> imageIndices;

        for (auto& imageToPresent : imagesToPresent)
        {
            swapchains.push_back(imageToPresent.swapchain);
            imageIndices.push_back(imageToPresent.imageIndex);
        }

        VkPresentInfoKHR presentInfo =
        {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            nullptr,
            static_cast<uint32_t>(renderingSemaphores.size()),
            renderingSemaphores.data(),
            static_cast<uint32_t>(swapchains.size()),
            swapchains.data(),
            imageIndices.data(),
            nullptr
        };

        VK_CHECK_RESULT(vkQueuePresentKHR(queue, &presentInfo));

        return true;
    }

    void DestroySwapchain(VkDevice device,
        VkSwapchainKHR& swapchain)
    {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void DestroyPresentationSurface(VkInstance instance,
        VkSurfaceKHR presentationSurface)
    {
        vkDestroySurfaceKHR(instance, presentationSurface, nullptr);
    }
}