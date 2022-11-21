#include "LogicalDevice.h"

namespace vk
{
    bool CreateLogicalDevice(VkPhysicalDevice physicalDevice,
        std::vector<const char*> desiredExtensions,
        std::vector<const char*> validationLayers,
        std::vector<QueueInfo> queuesInfo,
        VkPhysicalDeviceFeatures* desiredFeatures,
        bool isDebugModeEnabled,
        VkDevice& logicalDevice)
    {
        std::vector<VkExtensionProperties> availableExtensions;
        if (!CheckAvailableDeviceExtensions(physicalDevice, availableExtensions))
        {
            return false;
        }

        for (auto& ext : desiredExtensions)
        {
            if (!IsExtensionSupported(availableExtensions, ext))
            {
                std::string extStr = ext;
                WARN_LOG("Extension: " + extStr + " isnt supported by phisical device");
                return false;
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (auto& info : queuesInfo)
        {
            queueCreateInfos.push_back({
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                nullptr,
                0,
                info.index,
                static_cast<uint32_t>(info.priorities.size()),
                info.priorities.data()
                });
        }

        VkDeviceCreateInfo deviceInfo =
        {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(queueCreateInfos.size()),
            queueCreateInfos.data(),
            isDebugModeEnabled ? static_cast<uint32_t>(validationLayers.size()) : 0,
            isDebugModeEnabled ? validationLayers.data() : nullptr,
            static_cast<uint32_t>(desiredExtensions.size()),
            desiredExtensions.data(),
            desiredFeatures
        };

        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice));

        if (logicalDevice == VK_NULL_HANDLE)
        {
            CRITICAL_LOG("Failed to create logical device");
            return false;
        }

        return true;
    }

    void GetDeviceQueue(VkDevice device,
        uint32_t queueFamilyIndex,
        uint32_t queueIndex,
        VkQueue& queue)
    {
        vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &queue);
    }

    bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(VkInstance instance, 
        std::vector<const char*> desiredExtensions,
        std::vector<const char*> validationLayers,
        bool isDebugModeEnabled, 
        VkDevice& logicalDevice, 
        VkQueue& graphicsQueue, 
        VkQueue& computeQueue)
    {
        std::vector<VkPhysicalDevice> physicalDevices;
        EnumerateAvailablePhysicalDevices(instance, physicalDevices);

        for (auto& gpu : physicalDevices)
        {
            VkPhysicalDeviceFeatures features;
            VkPhysicalDeviceProperties properties;
            GetFeaturesAndPropertiesOfPhysicalDevice(gpu, features, properties);

            if (!features.geometryShader)
            {
                continue;
            }
            else
            {
                features.geometryShader = VK_TRUE;
            }

            uint32_t graphicsQueueFamilyIndex;
            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(gpu, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex))
            {
                continue;
            }

            uint32_t computeQueueFamilyIndex;
            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(gpu, VK_QUEUE_COMPUTE_BIT, computeQueueFamilyIndex))
            {
                continue;
            }

            std::vector<QueueInfo> requestedQueues = { {graphicsQueueFamilyIndex, {1.0f} }  };
            if (graphicsQueueFamilyIndex != computeQueueFamilyIndex)
            {
                requestedQueues.push_back({ computeQueueFamilyIndex, {1.0f} });
            }

            if (CreateLogicalDevice(gpu, desiredExtensions, validationLayers, requestedQueues, &features,
                isDebugModeEnabled, logicalDevice))
            {
                continue;
            }

            GetDeviceQueue(logicalDevice, computeQueueFamilyIndex, 0, computeQueue);
            GetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, graphicsQueue);
            return true;
        }

        return false;
    }

    void DestroyLogicalDevice(VkDevice& device)
    {
        vkDestroyDevice(device, nullptr);
    }
}