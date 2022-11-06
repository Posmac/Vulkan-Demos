#include "Instance.h"

namespace vk
{
    Instance::Instance()
    {

    }

    void Instance::Destroy()
    {
        if(instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance, nullptr);
        }
    }

    VkInstance Instance::GetInstance() const
    {
        return instance;
    }

    void Instance::CreateInstance(const VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo, bool isDebugModeEnabled,
                                  const std::vector<const char*>& usedValidationLayers,
                                  const std::vector<VkLayerProperties>& availableLayers,
                                  const std::vector<const char*>& usedInstanceExtensions)
    {
        GetAvailableInstanceExtensions();

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

        if(isDebugModeEnabled)
        {
            createInfo.pNext = &debugMessengerInfo;

            for(const char* layerName : usedValidationLayers)
            {
                for (const auto& layerProperties : availableLayers) 
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0) 
                    {
                        LOG_INFO("Instance layer " + std::string(layerName) + " is supported");
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

        for(const char* layerName : usedInstanceExtensions)
        {
            for (const auto& layerProperties : availableInstanceExtensions) 
            {
                if (strcmp(layerName, layerProperties.extensionName) == 0) 
                {
                    LOG_INFO("Instance extension " + std::string(layerName) + " is supported");
                }
            }
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(usedInstanceExtensions.size());
        createInfo.ppEnabledExtensionNames = usedInstanceExtensions.data();

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create instance");
        }
    }
    
    void Instance::GetAvailableInstanceExtensions()
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
}
