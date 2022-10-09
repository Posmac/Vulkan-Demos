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
        vkDestroyDevice(device, nullptr);
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

            for(const char* layerName : usedValidationLayers)
            {
                for (const auto& layerProperties : availableLayers) 
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0) 
                    {
                        LOG_INFO("\nLayer " + std::string(layerName) + " is supported\n");
                        break;
                    }
                }
            }

            createInfo.enabledLayerCount = static_cast<uint32_t>(usedValidationLayers.size());
            createInfo.ppEnabledLayerNames = usedValidationLayers.data();
        }
        else
        {
            createInfo.pNext = nullptr;
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        std::vector<const char*> extNames;
        extNames.reserve(availableInstanceExtensionsCount);

        for(const auto& ext : availableInstanceExtensions)
        {
            extNames.push_back(ext.extensionName);
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
        createInfo.ppEnabledExtensionNames = extNames.data();

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create instance");
        }
    }

    void VulkanApplication::pickUpPhysicalDevice()
    {
        uint32_t availablePhysicalDevices = 0;
        vkEnumeratePhysicalDevices(instance, &availablePhysicalDevices, nullptr);

        if(availablePhysicalDevices == 0)
        {
            LOG_INFO("Failed to querry any physical device on system");
        }

        std::vector<VkPhysicalDevice> devices(availablePhysicalDevices);
        vkEnumeratePhysicalDevices(instance, &availablePhysicalDevices, devices.data());

        for(const auto& device : devices)
        {
            if(isDeviceIsSuitable(device))
            {
                physicalDevice = device;
                return;
            }
        }
        LOG_INFO("Failed to find suitable physical devices, picked first one");
        physicalDevice = devices[0];
    }

    bool VulkanApplication::isDeviceIsSuitable(VkPhysicalDevice device)
    {
        uint32_t availableDeviceExtensions;
		std::vector<VkExtensionProperties> deviceExt;
		VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties props;
		uint32_t queueFamilies;
		std::vector<VkQueueFamilyProperties> familyProps;

        vkEnumerateDeviceExtensionProperties(device, nullptr, &availableDeviceExtensions, nullptr);

        deviceExt.resize(availableDeviceExtensions);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &availableDeviceExtensions, deviceExt.data());

        vkGetPhysicalDeviceFeatures(device, &features);
        vkGetPhysicalDeviceProperties(device, &props);

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilies, nullptr);
        familyProps.resize(queueFamilies);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilies, familyProps.data());

        VkQueueFlagBits flags = VK_QUEUE_GRAPHICS_BIT;
        for(int i = 0; i < queueFamilies; i++)
        {
            if(familyProps[i].queueCount > 0 && (familyProps[i].queueFlags & flags))
            {
                info.graphicsFamily = i;
                return true;
            }
        }

        return false;
    }

    void VulkanApplication::createLogicalDevice()
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
                    LOG_INFO("\nLayer " + std::string(extName) + " is supported\n");
                    break;
                }
            }
        }

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfo.queueFamilyIndex = info.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;

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
    }
}
