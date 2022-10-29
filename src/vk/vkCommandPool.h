#pragma once

#include "Core.h"

namespace vk
{
    class CVkCommandPool
    {
    public:
        CVkCommandPool();
        void Destroy();
        void CreateCommandPool(const VkDevice &device, uint32_t graphicsQueue);
        void ResetCommandPool(const VkDevice &device, VkCommandPoolResetFlags resetFlags);
    private:
        VkCommandPool commandPool;
    };
}