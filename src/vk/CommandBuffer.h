#pragma once

#include "Core.h"

namespace vk
{
    class CommandBuffer
    {
    public:
        CommandBuffer();
        void Destroy();
        std::vector<VkCommandBuffer>& CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, int count);
        void BeginCommandBufferRecord(VkCommandBufferUsageFlags flags, VkCommandBuffer buffer);
        void EndCommandBufferRecord(VkCommandBuffer buffer);
        void ResetCommandBuffer(VkCommandBuffer buffer, VkCommandBufferResetFlags resetFlags);
        void CreateVkSemaphore(VkDevice device, VkSemaphore& semaphore);
        void CreateFence(VkDevice device, VkFenceCreateFlagBits flag, VkFence& fence);
    private:
        std::vector<VkCommandBuffer> commandBuffers;
    };
}