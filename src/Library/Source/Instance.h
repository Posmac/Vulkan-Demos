#pragma once

#include "Library/Core/Core.h"
#include "DebugMesenger.h"

namespace vk
{
    bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties>& availableExtensions);

    bool IsExtensionSupported(std::vector<VkExtensionProperties>& availableExtensions, 
        const char* extension);

    bool CreateVulkanInstance(std::vector<const char*>& desiredExtensions,
        std::vector<const char*> desiredValidationLayers,
        bool isDebugModeEnabled,
        const char* appName,
        VkInstance& instance);

    void DestroyInstance(VkInstance& instance);
}