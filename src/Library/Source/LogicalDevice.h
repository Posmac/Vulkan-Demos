#pragma once

#include "Library/Core/Core.h"
#include "Library/Structs/QueueInfo.h"
#include "Instance.h"
#include "PhysicalDevice.h"

namespace vk
{
    bool CreateLogicalDevice(VkPhysicalDevice physicalDevice,
        std::vector<const char*> desiredExtensions,
        std::vector<const char*> validationLayers,
        std::vector<QueueInfo> queuesInfo,
        VkPhysicalDeviceFeatures* desiredFeatures,
        bool isDebugModeEnabled,
        VkDevice& logicalDevice);

    void GetDeviceQueue(VkDevice device, 
        uint32_t queueFamilyIndex, 
        uint32_t queueIndex, 
        VkQueue& queue);

    bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(VkInstance instance,
        std::vector<const char*> desiredExtensions,
        std::vector<const char*> validationLayers,
        bool isDebugModeEnabled,
        VkDevice& logicalDevice,
        VkQueue& graphicsQueue,
        VkQueue& computeQueue);

    void DestroyLogicalDevice(VkDevice& device);
}