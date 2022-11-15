#pragma once

#include "vulkan/vulkan.h"

namespace vk
{
    struct ViewportInfo
    {
        std::vector<VkViewport> viewPorts;
        std::vector<VkRect2D> scissors;
    };

    struct ShaderStageParams
    {
        VkShaderStageFlagBits shaderStage;
        VkShaderModule shaderModule;
        char const* entryPoint;
        VkSpecializationInfo const* specialisationInfo;
    };
}