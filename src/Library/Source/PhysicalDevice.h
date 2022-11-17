#pragma once

#include "Library/Core/Core.h"
#include "Library/Structs/QueueInfo.h"

namespace vk
{
    bool EnumerateAvailablePhysicalDevices(VkInstance instance,
        std::vector<VkPhysicalDevice>& availableDevices);

    bool CheckAvailableDeviceExtensions(VkPhysicalDevice device,
        std::vector<VkExtensionProperties>& availableExtensions);

    void GetFeaturesAndPropertiesOfPhysicalDevice(VkPhysicalDevice device,
        VkPhysicalDeviceFeatures& features,
        VkPhysicalDeviceProperties& properties);

    bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice device,
        std::vector<VkQueueFamilyProperties>& queueFamilies);

    bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice device,
        VkQueueFlags desiredCapabilities,
        uint32_t& queueFamilyIndex);
}