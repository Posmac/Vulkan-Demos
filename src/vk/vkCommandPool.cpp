#include "vkCommandPool.h"

namespace vk
{
    CVkCommandPool::CVkCommandPool()
    {

    }

    void CVkCommandPool::Destroy()
    {
        
    }

    void CVkCommandPool::CreateCommandPool(const VkDevice &device, uint32_t graphicsQueue)
    {
        VkCommandPoolCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = graphicsQueue;

        auto result = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);

        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create command pool");
        }
    }

    void CVkCommandPool::ResetCommandPool(const VkDevice &device, VkCommandPoolResetFlags resetFlags)
    {
        auto result = vkResetCommandPool(device, commandPool, resetFlags);
        if(result != VK_SUCCESS)
        {
            LOG_INFO("Failed to reset Command Pool");
        }   
    }
}