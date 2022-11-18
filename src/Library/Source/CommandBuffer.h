#pragma once

#include "Library/Core/Core.h"
#include "Library/Platform/WindowParams.h"
#include "Library/Structs/Image.h"
#include "Library/Structs/Semaphore.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"

namespace vk
{
    void CreateCommandPool(VkDevice device,
        VkCommandPoolCreateFlags poolCreateFlags,
        uint32_t queueFamily,
        VkCommandPool& commandPool);

    void AllocateCommandBuffers(VkDevice device,
        VkCommandPool commandPool,
        VkCommandBufferLevel level,
        uint32_t count,
        std::vector<VkCommandBuffer>& commandBuffers);

    void BeginCommandBufferRecordingOperation(VkCommandBuffer commandBuffer,
        VkCommandBufferUsageFlags usage,
        VkCommandBufferInheritanceInfo* secondaryCommandBufferInfo);

    void EndCommandBufferRecordingOperation(VkCommandBuffer commandBuffer);

    void ResetCommandBuffer(VkCommandBuffer commandBuffer,
        bool releaseResources);

    void ResetCommandPool(VkDevice device,
        VkCommandPool pool,
        bool releaseResources);

    void CreateVkSemaphore(VkDevice device,
        VkSemaphore& semaphore);

    void CreateVkFence(VkDevice device,
        bool signaled,
        VkFence& fence);

    bool WaitForFences(VkDevice device,
        const std::vector<VkFence>& fences,
        VkBool32 waitForAll,
        uint32_t timeout);

    void ResetFences(VkDevice device,
        const std::vector<VkFence>& fences);

    void SubmitCommandBuffersToQueue(VkQueue queue,
        const std::vector<WaitSemaphoreInfo>& waitInfo,
        const std::vector<VkCommandBuffer>& commandBuffers,
        const std::vector<VkSemaphore>& signalSemaphores,
        VkFence fence);

    void SyncronizeTwoCommandBuffers(VkQueue firstQueue,
        std::vector<WaitSemaphoreInfo> firstWaitInfo,
        std::vector<VkCommandBuffer> firstCommandBuffers,
        std::vector<WaitSemaphoreInfo> syncronizingSemaphores,
        VkQueue secondQueue,
        std::vector<VkCommandBuffer> secondCommandBuffers,
        std::vector<VkSemaphore> secondSignalSemaphores,
        VkFence secondFence);

    bool CheckIfProcessingOfSubmittedCommandBuffersHasFinished(VkDevice device,
        VkQueue queue,
        std::vector<WaitSemaphoreInfo> waitInfos,
        std::vector<VkCommandBuffer> commandBuffers,
        std::vector<VkSemaphore> signalSemaphores,
        VkFence fence,
        uint64_t timeout,
        VkResult& waitStatus);

    bool WaitForAllSumbittedCommandsToBeFinished(VkDevice device);

    void DestroyFence(VkDevice device,
        VkFence& fence);

    void DestroySemaphore(VkDevice device,
        VkSemaphore& semaphore);

    void FreeCommandBuffers(VkDevice device,
        VkCommandPool pool,
        std::vector<VkCommandBuffer>& commandBuffers);

    void DestroyCommandPool(VkDevice device,
        VkCommandPool& commandPool);
}