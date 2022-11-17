#pragma once

#include <algorithm>

#include "Library/Core/Core.h"
#include "Library/Platform/WindowParams.h"
#include "Library/Structs/Image.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"

namespace vk
{
    bool CreatePresentationSurface(VkInstance instance,
        WindowParameters windowParameters,
        VkSurfaceKHR& presentationSurface);

    bool SelectQueueFamilyThatSupportsPresentationToGivenSurface(VkPhysicalDevice device,
        VkSurfaceKHR presentationSurface,
        uint32_t& queueFamilyIndex);

    bool SelectDesiredPresentationMode(VkPhysicalDevice device,
        VkSurfaceKHR presentationSurface,
        VkPresentModeKHR desiredPresentMode,
        VkPresentModeKHR& presentMode);

    bool GetCapabilitiesOfPresentationSurface(VkPhysicalDevice device,
        VkSurfaceKHR presentationSurface,
        VkSurfaceCapabilitiesKHR& surfaceCapabilities);

    bool SelectingNumberOfSwapchainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities,
        uint32_t& imagesNumber);

    bool ChooseSizeOfSwapchainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities,
        VkExtent2D desiredSize,
        VkExtent2D& size);

    bool SelectDesiredUsageScenariosOfSwapChainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities,
        VkImageUsageFlags desiredUsage,
        VkImageUsageFlags& imageUsage);

    bool SelectTransformationOfSwapchainImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities,
        VkSurfaceTransformFlagBitsKHR desiredTransform,
        VkSurfaceTransformFlagBitsKHR& surfaceTransform);

    bool SelectFormatOfSwapChainImage(VkPhysicalDevice device,
        VkSurfaceKHR presentationSurface,
        VkSurfaceFormatKHR desiredSurfaceFormat,
        VkFormat& imageFormat,
        VkColorSpaceKHR& imageColorSpace);

    bool CreateSwapchain(VkDevice device,
        VkSurfaceKHR presentationSurface,
        uint32_t imagesCount,
        VkSurfaceFormatKHR surfaceFormat,
        VkExtent2D imageSize,
        VkImageUsageFlags imageUsage,
        VkSurfaceTransformFlagBitsKHR surfaceTransform,
        VkPresentModeKHR presentMode,
        VkSwapchainKHR& oldSwapchain,
        VkSwapchainKHR& swapchain);

    bool GetHandlesOfSwapChainImages(VkDevice device,
        VkSwapchainKHR swapchain,
        std::vector<VkImage>& swapchainImages);

    bool CreateSwapchainWithRGBA8FormatAndMailBoxPresentMode(VkDevice device,
        VkPhysicalDevice gpu,
        VkSurfaceKHR presentationSufrace,
        VkImageUsageFlags imageUsage,
        VkExtent2D& imageSize,
        VkFormat& imageFormat,
        VkSwapchainKHR& oldSwapchain,
        VkSwapchainKHR& swapchain,
        std::vector<VkImage>& swapchainImages);

    bool AcquireSwapchainImage(VkDevice device,
        VkSwapchainKHR swapchain,
        VkSemaphore semaphore,
        VkFence fence,
        uint32_t& imageIndex);

    bool PresentImage(VkQueue queue,
        std::vector<VkSemaphore> renderingSemaphores,
        std::vector<PresentInfo> imagesToPresent);

    void DestroySwapchain(VkDevice device,
        VkSwapchainKHR& swapchain);

    void DestroyPresentationSurface(VkInstance instance,
        VkSurfaceKHR presentationSurface);

}