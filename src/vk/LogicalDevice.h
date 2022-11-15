#pragma once

#include "Core.h"
#include "QueueInfo.h"

namespace vk
{
    class LogicalDevice
    {
    public:
        LogicalDevice();
        void Destroy();
        VkDevice GetDevice() const;
        VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice,
            std::vector<const char*> usedDeviceExtensions,
            std::vector<const char*> usedValidationLayers,
            bool computeQueueIndexEqualToGraphicsQueueIndex,
            QueuesInfo info,
            bool debugModeEnabled);
    private:
        VkDevice device;

    };
}