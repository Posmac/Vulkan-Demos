#pragma once

#include "Core.h"

namespace vk
{
    class CommandBuffer
    {
    public:
        CommandBuffer();
        void Destroy();
        void CreateCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, int count);
        void BeginCommandBufferRecord(VkCommandBufferUsageFlags flags, VkCommandBuffer buffer);
        void EndCommandBufferRecord(VkCommandBuffer buffer);
        void ResetCommandBuffer(VkCommandBuffer buffer, VkCommandBufferResetFlags resetFlags);
        void CreateSemaphore(const VkDevice& device);
        void CreateFence(VkDevice const &device, VkFenceCreateFlagBits flag);
    private:
        std::vector<VkCommandBuffer> commandBuffers;
        VkSemaphore semaphore;
        VkFence fence;
    };
}