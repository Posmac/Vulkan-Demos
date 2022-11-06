#include "VertexDiffuse.h"

namespace vk
{

    VertexDiffuseExample::VertexDiffuseExample(const std::vector<const char*>& instanceLayers,
        const std::vector<const char*>& deviceLayers,
        const std::vector<const char*>& extensions,
        bool isDebugModeEnabled)
    {
        //Get available instance layer properties
        std::vector<VkLayerProperties> availableLayerProperties;
        uint32_t availableLayerPropertiesCount;

        EnumerateAvailableLayerProperties(availableLayerPropertiesCount, availableLayerProperties,
            isDebugModeEnabled);

        instance.CreateInstance(debugMessenger.GetDebugUtilsMessengerInfo(), isDebugModeEnabled,
            instanceLayers, availableLayerProperties, extensions);
    }
    void VertexDiffuseExample::EnumerateAvailableLayerProperties(uint32_t& availableLayersCount,
        std::vector<VkLayerProperties>& availableLayerProperties, bool isDebugModeEnabled)
    {
        if (!isDebugModeEnabled)
        {
            LOG_INFO("Debug mode disabled, no layers will be enabled");
            return;
        }

        VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr));
        if (availableLayersCount == 0)
        {
            LOG_INFO("Cannot querry layers");
        }

        availableLayerProperties.resize(availableLayersCount);
        VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&availableLayersCount,
            availableLayerProperties.data()));
    }
}

