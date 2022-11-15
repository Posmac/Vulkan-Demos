#pragma once

#include "Core.h"

namespace vk
{
    class CommandPool
    {
    public:
        CommandPool();
        void Destroy();
        VkCommandPool CreateCommandPool(VkDevice device, uint32_t graphicsQueue);
        void ResetCommandPool(VkDevice device, VkCommandPoolResetFlags resetFlags);
    private:
        VkCommandPool commandPool;
    };
}