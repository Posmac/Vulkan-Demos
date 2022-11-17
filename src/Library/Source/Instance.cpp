#include "Instance.h"

namespace vk
{
    bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties>& availableExtensions)
    {
        uint32_t availableExtensionsCount = 0;

        VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, nullptr));

        if (availableExtensionsCount == 0)
        {
            WARN_LOG("Could not get the number of instance extensions");
            return false;
        }

        availableExtensions.resize(availableExtensionsCount);

        VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, availableExtensions.data()));

        if (availableExtensions.size() == 0)
        {
            WARN_LOG("Could not enumerate available extensions");
            return false;
        }

        return true;
    }

    bool IsExtensionSupported(std::vector<VkExtensionProperties>& availableExtensions, 
        const char* desiredExtension)
    {
        for (auto& extension : availableExtensions)
        {
            if (strstr(extension.extensionName, desiredExtension))
            {
                return true;
            }
        }
        return false;
    }

    bool CreateVulkanInstance(std::vector<const char*>& desiredExtensions,
        std::vector<const char*> desiredValidationLayers,
        bool isDebugModeEnabled,
        const char* appName,
        VkInstance& instance)
    {
        std::vector<VkExtensionProperties> availableExtensions;
        if (!CheckAvailableInstanceExtensions(availableExtensions))
        {
            return false;
        }

        for (auto& extension : desiredExtensions)
        {
            if (!IsExtensionSupported(availableExtensions, extension))
            {
                std::string extname = extension;
                WARN_LOG("Extension with name: " + extname + "is not supported");
            }
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "VulkanFramework";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "VulkanEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(desiredExtensions.size());
        createInfo.ppEnabledExtensionNames = desiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT messenger{};
        GetDebugUtilsMessengerInfo(messenger);
        if (isDebugModeEnabled)
        {
            createInfo.pNext = &messenger;
            createInfo.enabledLayerCount = static_cast<uint32_t>(desiredValidationLayers.size());
            createInfo.ppEnabledLayerNames = desiredValidationLayers.data();
        }
        else
        {
            createInfo.pNext = nullptr;
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));
        if (instance == VK_NULL_HANDLE)
        {
            CRITICAL_LOG("Failed to create instance");
            return false;
        }

        return true;
    }

    void DestroyInstance(VkInstance& instance)
    {
        vkDestroyInstance(instance, nullptr);
    }
}
