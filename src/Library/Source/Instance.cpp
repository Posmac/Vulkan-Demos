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

        VkApplicationInfo appInfo =
        {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            appName,
            VK_MAKE_VERSION(1, 0, 0),
            "Vulkan Framework",
            VK_MAKE_VERSION(1, 0, 0,),
            VK_MAKE_VERSION(1, 0, 0,)
        };

        VkDebugUtilsMessengerCreateInfoEXT messenger;
        if (isDebugModeEnabled)
        {
            GetDebugUtilsMessengerInfo(messenger);
        }

        VkInstanceCreateInfo instanceInfo =
        {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            isDebugModeEnabled ? &messenger : nullptr,
            0,
            &appInfo,
            isDebugModeEnabled ? static_cast<uint32_t>(desiredValidationLayers.size()) : 0,
            isDebugModeEnabled ? desiredValidationLayers.data() : nullptr,
            static_cast<uint32_t>(desiredExtensions.size()),
            desiredExtensions.data()
        };

        VK_CHECK_RESULT(vkCreateInstance(&instanceInfo, nullptr, &instance));
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
