#pragma once

#include "Core.h"

namespace vk
{
    class CommandPool
    {
    public:
        CommandPool();
        void Destroy();
        void CreateCommandPool(const VkDevice &device, uint32_t graphicsQueue);
        void ResetCommandPool(const VkDevice &device, VkCommandPoolResetFlags resetFlags);
    private:
        VkCommandPool commandPool;
    };
}