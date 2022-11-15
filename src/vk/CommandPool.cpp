#include "CommandPool.h"

namespace vk
{
    CommandPool::CommandPool()
    {

    }

    void CommandPool::Destroy()
    {
        
    }

    VkCommandPool CommandPool::CreateCommandPool(VkDevice device, uint32_t graphicsQueue)
    {
        VkCommandPoolCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = graphicsQueue;

        VK_CHECK_RESULT(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));

        return commandPool;
    }

    void CommandPool::ResetCommandPool(VkDevice device, VkCommandPoolResetFlags resetFlags)
    {
        auto result = vkResetCommandPool(device, commandPool, resetFlags);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to reset Command Pool");
        }   
    }
}