#include "PhysicalDevice.h"

namespace vk
{
    bool EnumerateAvailablePhysicalDevices(VkInstance instance,
        std::vector<VkPhysicalDevice>& availableDevices)
    {
        uint32_t availableDevicesCount = 0;
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &availableDevicesCount, nullptr));

        if (availableDevicesCount == 0)
        {
            WARN_LOG("Could not get the number of available physical devices");
            return false;
        }

        availableDevices.resize(availableDevicesCount);
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &availableDevicesCount, availableDevices.data()));

        if (availableDevices.data() == 0)
        {
            WARN_LOG("Could not enumerate physical devices");
            return false;
        }

        return true;
    }

    bool CheckAvailableDeviceExtensions(VkPhysicalDevice device,
        std::vector<VkExtensionProperties>& availableExtensions)
    {
        uint32_t extensionsCount = 0;
        VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr));
        if (extensionsCount == 0)
        {
            WARN_LOG("Count not get the number of device extensions");
            return false;
        }

        availableExtensions.resize(extensionsCount);
        VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data()));

        if (availableExtensions.size() == 0)
        {
            WARN_LOG("Could not enumerate device extensions");
            return false;
        }

        return true;
    }

    void GetFeaturesAndPropertiesOfPhysicalDevice(VkPhysicalDevice device,
        VkPhysicalDeviceFeatures& features,
        VkPhysicalDeviceProperties& properties)
    {
        vkGetPhysicalDeviceFeatures(device, &features);
        vkGetPhysicalDeviceProperties(device, &properties);
    }

    bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice device,
        std::vector<VkQueueFamilyProperties>& queueFamilies)
    {
        uint32_t queueFamiliesCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);

        if (queueFamiliesCount == 0)
        {
            WARN_LOG("Could not get the number of queue families");
            return false;
        }

        queueFamilies.resize(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());

        if (queueFamilies.size() == 0)
        {
            WARN_LOG("Could not acquire properties of queue families");
            return false;
        }

        return true;
    }

    bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice device,
        VkQueueFlags desiredCapabilities, 
        uint32_t& queueFamilyIndex)
    {
        std::vector<VkQueueFamilyProperties> queueFamilies;
        if (!CheckAvailableQueueFamiliesAndTheirProperties(device, queueFamilies))
        {
            return false;
        }

        for (int i = 0; i < queueFamilies.size(); i++)
        {
            if ((queueFamilies[i].queueCount > 0) &&
                (queueFamilies[i].queueFlags & desiredCapabilities) == desiredCapabilities)
            {
                queueFamilyIndex = static_cast<uint32_t>(i);
                return true;
            }
        }

        return false;
    }
}
