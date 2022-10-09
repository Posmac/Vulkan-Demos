#include "Vulkan.h"
#include "Core/Core.h"

namespace Vulkan
{
    VulkanApplication::VulkanApplication()
    {
#ifdef NDEBUG
    debugModeEnabled = false;
#else
    debugModeEnabled = true;
#endif
    }

    void VulkanApplication::querryAvailableLayers()
    {
        if(!debugModeEnabled)
        {
            LOG_INFO("Debug mode disabled, no layers will be enabled");
            return;
        }

        vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);
        if(availableLayersCount == 0)
        {
            LOG_INFO("Cannot querry layers");
        }

        availableLayers.resize(availableLayersCount);
        vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());

        for(const auto& layer : availableLayers)
        {
            LOG_INFO(layer.layerName);
        }
    }

    void VulkanApplication::getAvailableInstanceExtensions()
    {
        availableInstanceExtensionsCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionsCount, nullptr);

        if (availableInstanceExtensionsCount == 0)
        {
            LOG_INFO("Unable to get available instance extensions");
        }

        availableInstanceExtensions.resize(availableInstanceExtensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionsCount,
            availableInstanceExtensions.data());

        for (const auto& ext : availableInstanceExtensions)
        {
            LOG_INFO(ext.extensionName);
        }
    }

    void VulkanApplication::createDebugMesenger()
    {
        if(!debugModeEnabled)
        {
            LOG_INFO("Debug mode disabled, no debug object will be created");
            return;
        }

        auto debugInfo = getDebugUtilsMessengerInfo();
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        auto result = func(instance, &debugInfo, nullptr, &debugMessenger);
        if (result != VK_SUCCESS)
        {
            LOG_INFO("failed to create debug utils messenger");
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanApplication::getDebugUtilsMessengerInfo()
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.pNext = nullptr;
        debugInfo.flags = 0;//everytime 0, reserved for later use
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        debugInfo.pfnUserCallback = debugMesengerCallback;
        debugInfo.pUserData = nullptr;

        return debugInfo;
    }

    void VulkanApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void VulkanApplication::clean()
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    void VulkanApplication::createInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "Vulkan cookbook";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vulkan";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;

        if(debugModeEnabled)
        {
            auto debugInfo = getDebugUtilsMessengerInfo();
            createInfo.pNext = &debugInfo;

            std::vector<const char*> layers;
            getLayerNames(availableLayers, layers);

            createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
            createInfo.ppEnabledLayerNames = layers.data();
        }
        else
        {
            createInfo.pNext = nullptr;
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        std::vector<const char*> exts;
        getExtensionNames(availableInstanceExtensions, exts);

        createInfo.enabledExtensionCount = static_cast<uint32_t>(exts.size());
        createInfo.ppEnabledExtensionNames = exts.data();

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create instance");
        }
    }

    void VulkanApplication::getLayerNames(const std::vector<VkLayerProperties> &originalList,
        									std::vector<const char*> &returnList)
	{
        for(const auto& layer : originalList)
        {
            for(const auto& used : usedValidationLayers)
            {
                if(strcmp(layer.layerName, used))
                {
                    returnList.push_back(used);
                }
            }
        }
	}

    void VulkanApplication::getExtensionNames(const std::vector<VkExtensionProperties> &originalList,
        std::vector<const char*> &returnList)
	{
        returnList.reserve(originalList.size());
		for(const auto& ext : originalList)
        {
            returnList.push_back(ext.extensionName);
        }
	}
}
