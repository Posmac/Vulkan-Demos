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
        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
        }
    }

    VkDevice LogicalDevice::CreateLogicalDevice(VkPhysicalDevice physicalDevice,
        std::vector<const char*> usedDeviceExtensions,
        std::vector<const char*> usedValidationLayers,
        bool computeQueueIndexEqualToGraphicsQueueIndex,
        QueuesInfo info,
        bool debugModeEnabled)
    {
        uint32_t availableDeviceExtensions;
        std::vector<VkExtensionProperties> deviceExt;

        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableDeviceExtensions, nullptr);

        deviceExt.resize(availableDeviceExtensions);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableDeviceExtensions, deviceExt.data());

        for (const char* extName : usedDeviceExtensions)
        {
            for (const auto& ext : deviceExt)
            {
                if (strcmp(extName, ext.extensionName) == 0)
                {
                    LOG_INFO("Device extension " + std::string(extName) + " is supported");
                }
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
        const int size = computeQueueIndexEqualToGraphicsQueueIndex ? 1 : 2;
        queueCreateInfos.resize(size);

        float queuePriority = 1.0f;

        queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[0].pNext = nullptr;
        queueCreateInfos[0].flags = 0;

        queueCreateInfos[0].pQueuePriorities = &queuePriority;
        queueCreateInfos[0].queueFamilyIndex = info.graphicsFamily.value();
        queueCreateInfos[0].queueCount = 1;

        if (!computeQueueIndexEqualToGraphicsQueueIndex)
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

        if (debugModeEnabled)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(usedValidationLayers.size());
            createInfo.ppEnabledLayerNames = usedValidationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

        return device;
    }
}