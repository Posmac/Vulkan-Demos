#pragma once

#include "Library/Core/Core.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandBuffer.h"
#include "Resources.h"
#include "DescriptorSets.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Library/Structs/Pipeline.h"
#include "Library/Common/Tools.h"
#include "Library/Common/MeshLoader.h"
#include "Library/Structs/Semaphore.h"

namespace vk
{
    void ClearColorImage(VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout imageLayout,
        std::vector<VkImageSubresourceRange>& imageSubresourceRanges,
        VkClearColorValue& clearColor);

    void ClearDepthStencilImage(VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout layout,
        std::vector<VkImageSubresourceRange>& ranges,
        VkClearDepthStencilValue& clearValue);

    void ClearRenderPassAttachments(VkCommandBuffer commandBuffer,
        std::vector<VkClearAttachment>& attachments,
        std::vector<VkClearRect>& rects);

    void BindVertexBuffers(VkCommandBuffer commandBuffer,
        uint32_t firstBind,
        std::vector<VertexBufferParams>& bufferParams);

    void BindIndexBuffer(VkCommandBuffer commandBuffer,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkIndexType indexType);

    void ProvidePushConstants(VkCommandBuffer commandBuffer,
        VkPipelineLayout pipelineLayout,
        VkShaderStageFlags pipelineStages,
        uint32_t offset,
        uint32_t size,
        void* data);

    void SetViewportStateDynamically(VkCommandBuffer commandBuffer,
        uint32_t firstViewport,
        const std::vector<VkViewport>& viewports);

    void SetScissorsStateDynamically(VkCommandBuffer commandBuffer,
        uint32_t firstScissor,
        const std::vector<VkRect2D>& scissors);

    void SetLineWidthDynamically(VkCommandBuffer commandBuffer,
        float lineWidth);

    void SetDepthBiasStateDynamically(VkCommandBuffer commandBuffer,
        float constant,
        float clamp,
        float slope);

    void SetBlendConstantsStateDynamically(VkCommandBuffer commandBuffer,
        std::array<float, 4>& blendConstants);

    void DrawGeometry(VkCommandBuffer commandBuffer,
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance);

    void DrawIndexedGeometry(VkCommandBuffer commandBuffer,
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        uint32_t vertexOffset,
        uint32_t firstInstance);

    void DispatchComputeWork(VkCommandBuffer commandBuffer,
        uint32_t x,
        uint32_t y,
        uint32_t z);

    void ExecuteSecondaryCommandBufferInsidePrimary(VkCommandBuffer commandBuffer,
        std::vector<VkCommandBuffer>& secondaryCommandBuffers);

    void RecordCommandBufferForGeometryDrawAndViewportAndScirssorsDynStates(VkCommandBuffer commandBuffer,
        VkImage swapchainImage,
        uint32_t presentQueueFamilyIndex,
        uint32_t graphicsQueueFamilyIndex,
        VkRenderPass renderPass,
        VkFramebuffer framebuffer,
        VkExtent2D framebufferSize,
        std::vector<VkClearValue>& clearValues,
        VkPipeline graphicsPipeline,
        uint32_t firstVertexBufferBinds,
        std::vector<VertexBufferParams>& vertexBufferParams,
        VkPipelineLayout pipelineLayout,
        std::vector<VkDescriptorSet>& descriptorSets,
        uint32_t indexForFirstDescriptorSet,
        Mesh& model,
        uint32_t instanceCount,
        uint32_t firstInstance);

    void RecordCommandBufferOnMultipleThreads(std::vector<CommandBufferRecordingThreadParameters>& threadParams,
        VkQueue queue,
        std::vector<WaitSemaphoreInfo> waitInfos,
        std::vector<VkSemaphore> signalSemaphores,
        VkFence fence);

    void PrepareSingleFrameOfAnimation(VkDevice device,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkSwapchainKHR swapchain,
        VkExtent2D swapchainSize,
        std::vector<VkImageView>& swapchainImageViews,
        VkImageView depthAttachment,
        std::vector<WaitSemaphoreInfo>& waitInfos,
        VkSemaphore imageAcquiredSemaphore,
        VkSemaphore readyToPresentSemaphore,
        VkFence finishedDrawingFence,
        std::function<bool(VkCommandBuffer, uint32_t, VkFramebuffer)> recordCommandBuffer,
        VkCommandBuffer commandBuffer,
        VkRenderPass renderPass,
        VkFramebuffer framebuffer);

    void IncreasePerformanceThroughtIncreasingTheNumberOfSeparatelyRenderedFrames(VkDevice device,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkSwapchainKHR swapchain,
        VkExtent2D swapchainSize,
        std::vector<VkImageView>& swapchainImages,
        VkRenderPass renderPass,
        std::vector<WaitSemaphoreInfo>& waitInfos,
        std::function<bool(VkCommandBuffer, uint32_t, VkFramebuffer)> recordCommandBuffer,
        std::vector<FrameResources>& frameResources);
}