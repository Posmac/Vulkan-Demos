#include "vkCommandBuffer.h"

namespace vk
{
    CVkCommandBuffer::CVkCommandBuffer()
    {

    }

    void CVkCommandBuffer::Destroy()
    {

    }

    void CVkCommandBuffer::CreateCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, int count)
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(count);
        allocInfo.commandPool = commandPool;

        commandBuffers.resize(count);

        auto result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to allocate command buffers");
        }
    }

    void CVkCommandBuffer::BeginCommandBufferRecord(VkCommandBufferUsageFlags flags, VkCommandBuffer buffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags;
        beginInfo.pNext = nullptr;
        beginInfo.pInheritanceInfo = nullptr;

        auto result = vkBeginCommandBuffer(buffer, &beginInfo);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to begin cmd buffer record");
        }
    }

    void CVkCommandBuffer::EndCommandBufferRecord(VkCommandBuffer buffer)
    {
        auto result = vkEndCommandBuffer(buffer);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to end cmd buffer record");
        }
    }

    void CVkCommandBuffer::ResetCommandBuffer(VkCommandBuffer buffer, VkCommandBufferResetFlags resetFlags)
    {
        auto result = vkResetCommandBuffer(buffer, resetFlags);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to reset cmd buffer");
        }
    }

    void CVkCommandBuffer::CreateSemaphore(const VkDevice& device)
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0; // reserved for future use

        auto result = vkCreateSemaphore(device, &createInfo, nullptr, &semaphore);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create semaphore");
        }
    }

    void CVkCommandBuffer::CreateFence(VkDevice const &device, VkFenceCreateFlagBits flag)
    {
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = flag;

        auto result = vkCreateFence(device, &createInfo, nullptr, &fence);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create fence");
        }
    }
}
