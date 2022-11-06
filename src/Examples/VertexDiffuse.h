#pragma once

#include "vk/VulkanEntry.h"

namespace vk
{
    class VertexDiffuseExample
    {
    public:
        VertexDiffuseExample(const std::vector<const char*>& instanceLayers,
            const std::vector<const char*>& deviceLayers,
            const std::vector<const char*>& extensions,
            bool isDebugModeEnabled);
        void EnumerateAvailableLayerProperties(uint32_t &availableLayersCount, std::vector<VkLayerProperties> &availableLayerProperties,
            bool isDebugModeEnabled);
    private:
        DebugMessenger debugMessenger;
        Instance instance;
        LogicalDevice device;
        PhysicalDevice gpu;
    };
}