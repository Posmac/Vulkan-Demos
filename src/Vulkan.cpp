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
		VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties props;

		uint32_t queueFamilies;
		std::vector<VkQueueFamilyProperties> familyProps;

        vkGetPhysicalDeviceFeatures(device, &features);
        vkGetPhysicalDeviceProperties(device, &props);

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilies, nullptr);
        familyProps.resize(queueFamilies);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilies, familyProps.data());

        uint32_t graphicsFamily;
        uint32_t computeFamily;

        info = {};

        computeQueueIndexEqualToGraphicsQueueIndex = false;

        for(int i = 0; i < queueFamilies; i++)
        {
            if(familyProps[i].queueCount > 0 && features.geometryShader)
            {
                if( (familyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    info.computeFamily = i;
                }

                if((familyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                {
                    info.graphicsFamily = i;
                }

                if(info.isComplete())
                {
                    if(info.graphicsFamily.value() == info.computeFamily.value())
                    {
                        computeQueueIndexEqualToGraphicsQueueIndex = true;
                    }
                    return true;
                }
            }
        }

        LOG_INFO("Cannot find suitable physical device");

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
