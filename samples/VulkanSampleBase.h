#pragma once

#include "Library/Core/Core.h"
#include "Library/Platform/WindowParams.h"
#include "Library/Source/Instance.h"
#include "Library/Source/DebugMesenger.h"
#include "Library/Source/LogicalDevice.h"
#include "Library/Source/PhysicalDevice.h"
#include "Library/Source/Swapchain.h"
#include "Library/Source/CommandBuffer.h"
#include "Library/Source/Resources.h"
#include "Library/Source/RenderPass.h"
#include "Library/Source/Pipeline.h"
#include "Library/Source/Drawing.h"
#include "Library/Source/DescriptorSets.h"


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace vk
{
    class SampleBase
    {
    public:
        inline SampleBase() : isReady(false) {};
        inline virtual ~SampleBase() {};

        virtual bool Initialize(WindowParameters& windowParams,
            std::vector<const char*> validationLayer,
            std::vector<const char*> instanceExtensions,
            std::vector<const char*> deviceExtensions) = 0;

        virtual bool Draw() = 0;
        virtual bool Resize() = 0;
        virtual void Destroy() = 0;

        inline virtual bool IsReady() { return isReady; };
    protected:
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
        std::vector<VkDeviceMemory> depthImageMemory;
        std::vector<FrameResources> frameResources;
        uint32_t framesCount = 3;
        VkFormat depthFormat = VK_FORMAT_D16_UNORM;
        bool isReady;
    };


}