#pragma once

#include "Core.h"
#include "QueueInfo.h"

namespace vk
{
    class PhysicalDevice
    {
    public:
        PhysicalDevice();
        VkPhysicalDevice PickUpPhysicalDevice(const VkInstance& instance);
        VkPhysicalDevice GetGPU() const;
        bool IsQueuesIdentic() const;
        QueuesInfo GetQueuesInfo() const;
        VkQueue GetComputeQueue();
        VkQueue GetGraphicsQueue();
        QueuesInfo AcquireQueues(VkDevice device);
    private:
        bool IsDeviceIsSuitable(VkPhysicalDevice device);
    private:
        VkPhysicalDevice physicalDevice;
		QueuesInfo info;
		bool computeQueueIndexEqualToGraphicsQueueIndex;
		VkQueue graphicsQueue;
		VkQueue computeQueue;
    };
}