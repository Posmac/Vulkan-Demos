#pragma once

#include "vulkan/vulkan.h"

namespace vk
{
    struct WaitSemaphoreInfo
    {
        VkSemaphore Semaphore;
        VkPipelineStageFlags WaitingStage;
    };
}
