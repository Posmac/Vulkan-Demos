#pragma once

#include "vulkan/vulkan.h"

namespace vk
{
    struct CommandBufferRecordingThreadParameters 
    {
        VkCommandBuffer commandBuffer;
        std::function<bool(VkCommandBuffer)> recordingFunction;
    };

    struct WaitSemaphoreInfo
    {
        VkSemaphore Semaphore;
        VkPipelineStageFlags WaitingStage;
    };
}
