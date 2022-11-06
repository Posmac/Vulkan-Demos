#pragma once

#include "Core.h"

namespace vk
{
    class Instance
    {
    public:
        Instance();
        void Destroy();
        void CreateInstance(const VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo, bool isDebugModeEnabled,
                            const std::vector<const char*>& usedValidationLayers, 
                            const std::vector<VkLayerProperties>& availableLayers,
                            const std::vector<const char*>& usedInstanceExtensions);
        VkInstance GetInstance() const;
    private:
        void GetAvailableInstanceExtensions();
    private:
        VkInstance instance;

		uint32_t availableInstanceExtensionsCount;
		std::vector<VkExtensionProperties> availableInstanceExtensions;
    };
}