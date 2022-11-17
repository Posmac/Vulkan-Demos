#include "CommandBuffer.h"

namespace vk
{
    void CreateCommandPool(VkDevice device,
        VkCommandPoolCreateFlags poolCreateFlags, 
        uint32_t queueFamily, 
        VkCommandPool& commandPool)
    {
        VkCommandPoolCreateInfo poolInfo =
        {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            nullptr,
            poolCreateFlags,
            queueFamily
        };

        VK_CHECK_RESULT(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
    }

    void AllocateCommandBuffers(VkDevice device, VkCommandPool commandPool,
        VkCommandBufferLevel level, 
        uint32_t count, 
        std::vector<VkCommandBuffer>& commandBuffers)
    {
        commandBuffers.resize(count);

        VkCommandBufferAllocateInfo allocInfo =
        {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            nullptr,
            commandPool,
            level,
            count
        };

        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()))
    }

    void BeginCommandBufferRecordingOperation(VkCommandBuffer commandBuffer,
        VkCommandBufferUsageFlags usage, 
        VkCommandBufferInheritanceInfo* secondaryCommandBufferInfo)
    {
        VkCommandBufferBeginInfo beginInfo =
        {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            usage,
            secondaryCommandBufferInfo
        };

        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    void EndCommandBufferRecordingOperation(VkCommandBuffer commandBuffer)
    {
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
    }

    void ResetCommandBuffer(VkCommandBuffer commandBuffer,
        bool releaseResources)
    {
        VK_CHECK_RESULT(vkResetCommandBuffer(commandBuffer, releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0));
    }

    void ResetCommandPool(VkDevice device, 
        VkCommandPool pool, 
        bool releaseResources)
    {
        VK_CHECK_RESULT(vkResetCommandPool(device, pool,
            releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0))
    }

    void CreateVkSemaphore(VkDevice device, 
        VkSemaphore& semaphore)
    {
        VkSemaphoreCreateInfo semaphoreInfo =
        {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            nullptr,
            0
        };

        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore));
    }

    void CreateVkFence(VkDevice device, 
        bool signaled, 
        VkFence& fence)
    {
        VkFenceCreateInfo fenceInfo =
        {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            nullptr,
            signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u
        };

        VK_CHECK_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &fence));
    }

    bool WaitForFences(VkDevice device, 
        const std::vector<VkFence>& fences, 
        VkBool32 waitForAll, 
        uint32_t timeout)
    {
        if (fences.size() > 0)
        {
            VkResult result = vkWaitForFences(device,
                static_cast<uint32_t>(fences.size()), fences.data(), waitForAll, timeout);

            if (result != VK_SUCCESS)
            {
                WARN_LOG("Waiting for fence failed");
                return false;
            }

            return true;
        }

        return false;
    }

    void ResetFences(VkDevice device, 
        const std::vector<VkFence>& fences)
    {
        VK_CHECK_RESULT(vkResetFences(device, static_cast<uint32_t>(fences.size()), fences.data()));
    }

    void SubmitCommandBuffersToQueue(VkQueue queue, 
        std::vector<WaitSemaphoreInfo> waitInfo, 
        std::vector<VkCommandBuffer> commandBuffers, 
        std::vector<VkSemaphore> signalSemaphores, 
        VkFence fence)
    {
        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkPipelineStageFlags> waitSemaphoreStages;

        for (auto& info : waitInfo)
        {
            waitSemaphores.push_back(info.Semaphore);
            waitSemaphoreStages.push_back(info.WaitingStage);
        }

        VkSubmitInfo submitInfo =
        {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            nullptr,
            static_cast<uint32_t>(waitSemaphores.size()),
            waitSemaphores.data(),
            waitSemaphoreStages.data(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data(),
            static_cast<uint32_t>(signalSemaphores.size()),
            signalSemaphores.data()
        };

        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, & submitInfo, fence))
    }

    void SyncronizeTwoCommandBuffers(VkQueue firstQueue, 
        std::vector<WaitSemaphoreInfo> firstWaitInfo, 
        std::vector<VkCommandBuffer> firstCommandBuffers, 
        std::vector<WaitSemaphoreInfo> syncronizingSemaphores, 
        VkQueue secondQueue, 
        std::vector<VkCommandBuffer> secondCommandBuffers, 
        std::vector<VkSemaphore> secondSignalSemaphores, 
        VkFence secondFence)
    {
        std::vector<VkSemaphore> firstSignalSemaphores;
        for (auto& semaphoreInfo : syncronizingSemaphores)
        {
            firstSignalSemaphores.emplace_back(semaphoreInfo.Semaphore);
        }

        SubmitCommandBuffersToQueue(firstQueue, firstWaitInfo, firstCommandBuffers,
            firstSignalSemaphores, VK_NULL_HANDLE);

        SubmitCommandBuffersToQueue(secondQueue, syncronizingSemaphores, secondCommandBuffers,
            secondSignalSemaphores, secondFence);
    }

    bool CheckIfProcessingOfSubmittedCommandBuffersHasFinished(VkDevice device, 
        VkQueue queue, 
        std::vector<WaitSemaphoreInfo> waitInfos, 
        std::vector<VkCommandBuffer> commandBuffers, 
        std::vector<VkSemaphore> signalSemaphores, 
        VkFence fence, 
        uint64_t timeout, 
        VkResult& waitStatus)
    {
        SubmitCommandBuffersToQueue(queue, waitInfos, commandBuffers, signalSemaphores, fence);
        std::vector<VkFence> fences = { fence };
        return WaitForFences(device, fences, VK_FALSE, timeout);
    }

    bool WaitForAllSumbittedCommandsToBeFinished(VkDevice device)
    {
        VkResult res = vkDeviceWaitIdle(device);

        if (res != VK_SUCCESS)
        {
            WARN_LOG("Waiting for device idle failed");
        }

        return res == VK_SUCCESS;
    }


    void DestroyFence(VkDevice device, 
        VkFence& fence)
    {
        vkDestroyFence(device, fence, nullptr);
    }

    void DestroySemaphore(VkDevice device,
        VkSemaphore& semaphore)
    {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    void FreeCommandBuffers(VkDevice device, 
        VkCommandPool pool, 
        std::vector<VkCommandBuffer>& commandBuffers)
    {
        vkFreeCommandBuffers(device, pool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void DestroyCommandPool(VkDevice device, 
        VkCommandPool& commandPool)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
    }
}
