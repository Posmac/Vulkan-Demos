#include "LogicalDevice.h"

namespace vk
{
    LogicalDevice::LogicalDevice()
    {

    }

    VkDevice LogicalDevice::GetDevice() const
    {
        return device;
    }

    void LogicalDevice::Destroy()
    {
        if(device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
        }
    }

    void LogicalDevice::createLogicalDevice(const VkPhysicalDevice& physicalDevice,
                                            std::vector<const char*> usedDeviceExtensions,
                                            std::vector<const char*> usedValidationLayers,
                                            bool computeQueueIndexEqualToGraphicsQueueIndex, const QueuesInfo& info,
                                            bool debugModeEnabled, VkQueue graphicsQueue,
                                            VkQueue computeQueue)
    {
        uint32_t availableDeviceExtensions;
		std::vector<VkExtensionProperties> deviceExt;

        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableDeviceExtensions, nullptr);

        deviceExt.resize(availableDeviceExtensions);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableDeviceExtensions, deviceExt.data());

        for(const char* extName : usedDeviceExtensions)
        {
            for (const auto& ext : deviceExt) 
            {
                if (strcmp(extName, ext.extensionName) == 0) 
                {
                    LOG_INFO("Device extension " + std::string(extName) + " is supported");
                }
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos {};
        const int size = computeQueueIndexEqualToGraphicsQueueIndex ? 1 : 2;
        queueCreateInfos.resize(size);

        float queuePriority = 1.0f;

        queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[0].pNext = nullptr;
        queueCreateInfos[0].flags = 0;

        queueCreateInfos[0].pQueuePriorities = &queuePriority;
        queueCreateInfos[0].queueFamilyIndex = info.graphicsFamily.value();
        queueCreateInfos[0].queueCount = 1;

        if(!computeQueueIndexEqualToGraphicsQueueIndex)
        {
            queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[1].pNext = nullptr;
            queueCreateInfos[1].flags = 0;
            queueCreateInfos[1].pQueuePriorities = &queuePriority;
            queueCreateInfos[1].queueFamilyIndex = info.computeFamily.value();
            queueCreateInfos[1].queueCount = 1;
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        VkPhysicalDeviceFeatures features{};
        createInfo.pEnabledFeatures = &features;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(usedDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = usedDeviceExtensions.data();

        if(debugModeEnabled)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(usedValidationLayers.size());
            createInfo.ppEnabledLayerNames = usedValidationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create logical device");
        }

        vkGetDeviceQueue(device, info.graphicsFamily.value(), 0, &graphicsQueue);
        if(!computeQueueIndexEqualToGraphicsQueueIndex)
        {
            vkGetDeviceQueue(device, info.computeFamily.value(), 0, &computeQueue);
        }
        LOG_INFO("Queue families used indexes:");
        LOG_INFO("Graphics family queue index " + std::to_string(info.graphicsFamily.value()));
        LOG_INFO("Compute family queue index " + std::to_string(info.computeFamily.value()));
    }
}