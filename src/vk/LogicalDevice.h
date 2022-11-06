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
        void createLogicalDevice(const VkPhysicalDevice& physicalDevice, 
                                            std::vector<const char*> usedDeviceExtensions,
                                            std::vector<const char*> usedValidationLayers,
                                            bool computeQueueIndexEqualToGraphicsQueueIndex, 
                                            const QueuesInfo& info, bool debugModeEnabled, 
                                            VkQueue graphicsQueue, VkQueue computeQueue);
    private:
        VkDevice device;

    };
}