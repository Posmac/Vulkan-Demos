#include "CommandBuffer.h"

namespace vk
{
    CommandBuffer::CommandBuffer()
    {

    }

    void CommandBuffer::Destroy()
    {

    }

    std::vector<VkCommandBuffer>& CommandBuffer::CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, int count)
    {
        VkCommandBufferAllocateInfo allocInfo {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(count);
        allocInfo.commandPool = commandPool;

        commandBuffers.resize(count);

        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));

        return commandBuffers;
    }

    void CommandBuffer::BeginCommandBufferRecord(VkCommandBufferUsageFlags flags, VkCommandBuffer buffer)
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

    void CommandBuffer::EndCommandBufferRecord(VkCommandBuffer buffer)
    {
        auto result = vkEndCommandBuffer(buffer);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to end cmd buffer record");
        }
    }

    void CommandBuffer::ResetCommandBuffer(VkCommandBuffer buffer, VkCommandBufferResetFlags resetFlags)
    {
        auto result = vkResetCommandBuffer(buffer, resetFlags);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to reset cmd buffer");
        }
    }

    void CommandBuffer::CreateVkSemaphore(VkDevice device, VkSemaphore& semaphore)
    {
        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0; // reserved for future use

        VK_CHECK_RESULT(vkCreateSemaphore(device, &createInfo, nullptr, &semaphore));
    }

    void CommandBuffer::CreateFence(VkDevice device, VkFenceCreateFlagBits flag, 
        VkFence& fence)
    {
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = flag;

        VK_CHECK_RESULT(vkCreateFence(device, &createInfo, nullptr, &fence));
    }
}
