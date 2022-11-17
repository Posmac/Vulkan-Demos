#include "VulkanSample.h"

namespace vk
{
    bool Sample::InitVulkan(WindowParameters& windowParams,
        std::vector<const char*> validationLayer,
        std::vector<const char*> instanceExtensions,
        std::vector<const char*> deviceExtensions,
        VkPhysicalDeviceFeatures* deviceFeatures,
        bool useDepth,
        VkImageUsageFlags swapChainImageUsage,
        VkImageUsageFlags depthImageUsage)
    {
        //creating instance with layers and extensions
        CreateVulkanInstance(instanceExtensions, validationLayer, true, "Vulkan Framework", instance);

        //create debugger
        CreateDebugMesenger(instance, true, messenger);

        //creating surface
        CreatePresentationSurface(instance, windowParams, presentationSurface);

        //picking gpu
        std::vector<VkPhysicalDevice> physicalDevices;
        EnumerateAvailablePhysicalDevices(instance, physicalDevices);

        for (auto& gpu : physicalDevices)
        {
            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(gpu, VK_QUEUE_GRAPHICS_BIT, graphicsQueue.familyIndex))
            {
                continue;
            }

            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(gpu, VK_QUEUE_COMPUTE_BIT, computeQueue.familyIndex))
            {
                continue;
            }

            if (!SelectQueueFamilyThatSupportsPresentationToGivenSurface(gpu, presentationSurface, presentQueue.familyIndex))
            {
                continue;
            }

            std::vector<QueueInfo> requstedQueues = { {graphicsQueue.familyIndex, {1.0f}} };
            if (graphicsQueue.familyIndex != computeQueue.familyIndex)
            {
                requstedQueues.push_back({ computeQueue.familyIndex, {1.0f} });
            }
            if (graphicsQueue.familyIndex != presentQueue.familyIndex &&
                computeQueue.familyIndex != presentQueue.familyIndex)
            {
                requstedQueues.push_back({ presentQueue.familyIndex, {1.0f} });
            }

            if (!CreateLogicalDevice(gpu, deviceExtensions, validationLayer, requstedQueues,
                deviceFeatures, true, device))
            {
                continue;
            }

            physicalDevice = gpu;
            GetDeviceQueue(device, graphicsQueue.familyIndex, 0, graphicsQueue.handle);
            GetDeviceQueue(device, computeQueue.familyIndex, 0, graphicsQueue.handle);
            GetDeviceQueue(device, presentQueue.familyIndex, 0, graphicsQueue.handle);
            break;
        }

        if (device == VK_NULL_HANDLE)
        {
            WARN_LOG("No device was created");
            return false;
        }

        //creating command pool
        CreateCommandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue.familyIndex, commandPool);

        //preparing frameresources
        for (uint32_t i = 0; i < framesCount; i++)
        {
            std::vector<VkCommandBuffer> commandBuffer;
            VkSemaphore imageAcquiredSemaphore;
            VkSemaphore readyToPresentSemaphore;
            VkFence drawingFinishedFence;
            VkImageView depthAttachment;
            VkFramebuffer framebuffer;

            AllocateCommandBuffers(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                1, commandBuffer);
            CreateVkSemaphore(device, imageAcquiredSemaphore);
            CreateVkSemaphore(device, readyToPresentSemaphore);
            CreateVkFence(device, true, drawingFinishedFence);

            frameResources.push_back(
                {
                    commandBuffer[0],
                    std::move(imageAcquiredSemaphore),
                    std::move(readyToPresentSemaphore),
                    std::move(drawingFinishedFence),
                    std::move(depthAttachment),
                    std::move(framebuffer)
                }
            );
        }

        swapchain.format = VK_FORMAT_R8G8B8A8_UNORM;
        swapchain.size = { 800, 600 };
        swapchain.swapchain = VK_NULL_HANDLE;
        swapchain.images.clear();
        swapchain.imageViews.clear();
        depthImages.clear();
        depthImageViews.clear();
        depthImageMemory.clear();

        if (!CreateSwapchain(useDepth, swapChainImageUsage, depthImageUsage))
        {
            return false;
        }

        return true;
    }

    bool Sample::CreateSwapchain(bool useDepth, 
        VkImageUsageFlags swapChainImageUsage, 
        VkImageUsageFlags depthImageUsage)
    {
        WaitForAllSumbittedCommandsToBeFinished(device);

        isReady = false;

        VkSwapchainKHR oldswapchain = swapchain.swapchain;

        if (!CreateSwapchainWithRGBA8FormatAndMailBoxPresentMode(device, physicalDevice, presentationSurface,
            swapChainImageUsage, swapchain.size, swapchain.format, oldswapchain, swapchain.swapchain, swapchain.images))
        {
            return false;
        }

        swapchain.imageViews.resize(swapchain.images.size());
        for (int i = 0; i < swapchain.images.size(); i++)
        {
            CreateImageView(device, swapchain.images[i], VK_IMAGE_VIEW_TYPE_2D, swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT,
                swapchain.imageViews[i]);
        }

        depthImages.resize(framesCount);
        depthImageViews.resize(framesCount);
        depthImageMemory.resize(framesCount);

        if (useDepth)
        {
            for (int i = 0; i < framesCount; i++)
            {
                Create2DImageAndView(device, physicalDevice, depthFormat, swapchain.size, 1, 1,
                    VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
                    depthImages[i], depthImageMemory[i], depthImageViews[i]);
            }
        }

        isReady = true;
        return true;
    }

    bool Sample::Initialize(WindowParameters& windowParams,
        std::vector<const char*> validationLayer,
        std::vector<const char*> instanceExtensions,
        std::vector<const char*> deviceExtensions)
    {
        if (!InitVulkan(windowParams, validationLayer, instanceExtensions, deviceExtensions, nullptr,
           false, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
        {
            return false;
        }

        return true;
    }

    bool Sample::Draw()
    {
        return true;
    }

    bool Sample::Resize()
    {
        return true;
    }

    void Sample::Destroy()
    {
        WaitForAllSumbittedCommandsToBeFinished(device);
    }
}