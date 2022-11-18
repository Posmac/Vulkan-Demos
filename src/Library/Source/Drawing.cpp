#include "Drawing.h"

namespace vk
{
    void ClearColorImage(VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout imageLayout,
        std::vector<VkImageSubresourceRange>& imageSubresourceRanges,
        VkClearColorValue& clearColor)
    {
        vkCmdClearColorImage(commandBuffer, image, imageLayout,
            &clearColor, static_cast<uint32_t>(imageSubresourceRanges.size()), imageSubresourceRanges.data());
    }

    void ClearDepthStencilImage(VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout layout,
        std::vector<VkImageSubresourceRange>& ranges,
        VkClearDepthStencilValue& clearValue)
    {
        vkCmdClearDepthStencilImage(commandBuffer, image, layout,
            &clearValue, static_cast<uint32_t>(ranges.size()), ranges.data());
    }

    void ClearRenderPassAttachments(VkCommandBuffer commandBuffer,
        std::vector<VkClearAttachment>& attachments,
        std::vector<VkClearRect>& rects)
    {
        vkCmdClearAttachments(commandBuffer, static_cast<uint32_t>(attachments.size()), attachments.data(),
            static_cast<uint32_t>(rects.size()), rects.data());
    }

    void BindVertexBuffers(VkCommandBuffer commandBuffer,
        uint32_t firstBind,
        const std::vector<VertexBufferParams>& bufferParams)
    {
        std::vector<VkBuffer> buffers;
        std::vector<VkDeviceSize> offsets;
        for (auto& param : bufferParams)
        {
            buffers.push_back(param.buffer);
            offsets.push_back(param.memoryOffset);
        }

        vkCmdBindVertexBuffers(commandBuffer, firstBind, static_cast<uint32_t>(bufferParams.size()),
            buffers.data(), offsets.data());
    }

    void BindIndexBuffer(VkCommandBuffer commandBuffer,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkIndexType indexType)
    {
        vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }

    void ProvidePushConstants(VkCommandBuffer commandBuffer,
        VkPipelineLayout pipelineLayout,
        VkShaderStageFlags pipelineStages,
        uint32_t offset,
        uint32_t size,
        void* data)
    {
        vkCmdPushConstants(commandBuffer, pipelineLayout, pipelineStages, offset, size, data);
    }

    void SetViewportStateDynamically(VkCommandBuffer commandBuffer,
        uint32_t firstViewport,
        const std::vector<VkViewport>& viewports)
    {
        vkCmdSetViewport(commandBuffer, firstViewport, static_cast<uint32_t>(viewports.size()), viewports.data());
    }

    void SetScissorsStateDynamically(VkCommandBuffer commandBuffer,
        uint32_t firstScissor,
        const std::vector<VkRect2D>& scissors)
    {
        vkCmdSetScissor(commandBuffer, firstScissor, static_cast<uint32_t>(scissors.size()), scissors.data());
    }

    void SetLineWidthDynamically(VkCommandBuffer commandBuffer,
        float lineWidth)
    {
        vkCmdSetLineWidth(commandBuffer, lineWidth);
    }

    void SetDepthBiasStateDynamically(VkCommandBuffer commandBuffer,
        float constant,
        float clamp,
        float slope)
    {
        vkCmdSetDepthBias(commandBuffer, constant, clamp, slope);
    }

    void SetBlendConstantsStateDynamically(VkCommandBuffer commandBuffer,
        std::array<float, 4>& blendConstants)
    {
        vkCmdSetBlendConstants(commandBuffer, blendConstants.data());
    }

    void DrawGeometry(VkCommandBuffer commandBuffer,
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance)
    {
        vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void DrawIndexedGeometry(VkCommandBuffer commandBuffer,
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        uint32_t vertexOffset,
        uint32_t firstInstance)
    {
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void DispatchComputeWork(VkCommandBuffer commandBuffer,
        uint32_t x,
        uint32_t y,
        uint32_t z)
    {
        vkCmdDispatch(commandBuffer, x, y, z);
    }

    void ExecuteSecondaryCommandBufferInsidePrimary(VkCommandBuffer commandBuffer,
        std::vector<VkCommandBuffer>& secondaryCommandBuffers)
    {
        vkCmdExecuteCommands(commandBuffer, static_cast<uint32_t>(secondaryCommandBuffers.size()),
            secondaryCommandBuffers.data());
    }

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
        uint32_t firstInstance)
    {
        BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            nullptr);

        if (presentQueueFamilyIndex != graphicsQueueFamilyIndex)
        {
            ImageTransition imageTransitionBeforeDrawing =
            {
                swapchainImage,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                presentQueueFamilyIndex,
                graphicsQueueFamilyIndex,
                VK_IMAGE_ASPECT_COLOR_BIT
            };

            SetImageMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { imageTransitionBeforeDrawing });
        }

        BeginRenderPass(commandBuffer, renderPass, framebuffer, { {0, 0}, framebufferSize },
            clearValues, VK_SUBPASS_CONTENTS_INLINE);

        BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport =
        {
            0.0f,
            0.0f,
            static_cast<float>(framebufferSize.width),
            static_cast<float>(framebufferSize.height),
            0.0f,
            1.0f
        };

        SetViewportStateDynamically(commandBuffer, 0, { viewport });

        VkRect2D scissor =
        {
            {0 , 0},
            {framebufferSize.width, framebufferSize.height}
        };
        SetScissorsStateDynamically(commandBuffer, 0, { scissor });

        BindVertexBuffers(commandBuffer, firstVertexBufferBinds, vertexBufferParams);

        BindDescitorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, indexForFirstDescriptorSet,
            descriptorSets, {});

        for (int i = 0; i < model.meshes.size(); i++)
        {
            DrawGeometry(commandBuffer, model.meshes[i].vertexCount, instanceCount, model.meshes[i].vertexOffset,
                firstInstance);
        }

        EndRenderPass(commandBuffer);

        if (presentQueueFamilyIndex != graphicsQueueFamilyIndex)
        {
            ImageTransition imageTransitionBeforeDrawing =
            {
                swapchainImage,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                graphicsQueueFamilyIndex,
                presentQueueFamilyIndex,
                VK_IMAGE_ASPECT_COLOR_BIT
            };

            SetImageMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { imageTransitionBeforeDrawing });
        }

        EndCommandBufferRecordingOperation(commandBuffer);
    }

    void RecordCommandBufferOnMultipleThreads(std::vector<CommandBufferRecordingThreadParameters>& threadParams,
        VkQueue queue,
        std::vector<WaitSemaphoreInfo> waitInfos,
        std::vector<VkSemaphore> signalSemaphores,
        VkFence fence)
    {
        std::vector<std::thread> threads(threadParams.size());
        for (int i = 0; i < threadParams.size(); i++)
        {
            threads[i] = std::thread(threadParams[i].recordingFunction, threadParams[i].commandBuffer);
        }

        std::vector<VkCommandBuffer> commandBuffers(threadParams.size());
        for (int i = 0; i < threadParams.size(); i++)
        {
            threads[i].join();
            commandBuffers.push_back(threadParams[i].commandBuffer);
        }

        SubmitCommandBuffersToQueue(queue, waitInfos, commandBuffers, signalSemaphores, fence);
    }

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
        VkFramebuffer framebuffer)
    {
        uint32_t imageIndex;
        AcquireSwapchainImage(device, swapchain, imageAcquiredSemaphore, VK_NULL_HANDLE, imageIndex);

        std::vector<VkImageView> attachments = { swapchainImageViews[imageIndex] };
        if (depthAttachment != VK_NULL_HANDLE)
        {
            attachments.push_back(depthAttachment);
        }

        CreateFramebuffer(device, renderPass, attachments, swapchainSize.width, swapchainSize.height,
            1, framebuffer);

        if (!recordCommandBuffer(commandBuffer, imageIndex, framebuffer))
        {
            WARN_LOG("Failed to record command buffer");
            return;
        }

        std::vector<WaitSemaphoreInfo> waitInfo = waitInfos;
        WaitSemaphoreInfo newInfo = { imageAcquiredSemaphore,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        waitInfo.push_back(newInfo);

        SubmitCommandBuffersToQueue(graphicsQueue, waitInfo, { commandBuffer },
            { readyToPresentSemaphore }, finishedDrawingFence);

        PresentInfo presentInfo = { swapchain, imageIndex };

        PresentImage(presentQueue, { readyToPresentSemaphore }, { presentInfo });
    }

    void IncreasePerformanceThroughtIncreasingTheNumberOfSeparatelyRenderedFrames(VkDevice device,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkSwapchainKHR swapchain,
        VkExtent2D swapchainSize,
        std::vector<VkImageView>& swapchainImages,
        VkRenderPass renderPass,
        std::vector<WaitSemaphoreInfo>& waitInfos,
        std::function<bool(VkCommandBuffer, uint32_t, VkFramebuffer)> recordCommandBuffer,
        std::vector<FrameResources>& frameResources)
    {
        static uint32_t frameIndex = 0;
        FrameResources& currentFrame = frameResources[frameIndex];

        if (!WaitForFences(device, { currentFrame.drawingFinishedFence }, false, 2000000000))
        {
            return;
        }

        ResetFences(device, { currentFrame.drawingFinishedFence });

        PrepareSingleFrameOfAnimation(device, graphicsQueue, presentQueue, swapchain, swapchainSize,
            swapchainImages, currentFrame.depthAttachment, waitInfos, currentFrame.imageAcquiredSemaphore,
            currentFrame.readyToPresentSemaphore, currentFrame.drawingFinishedFence, recordCommandBuffer, 
            currentFrame.commandBuffer, renderPass, currentFrame.framebuffer);

        frameIndex = (frameIndex + 1) % frameResources.size();
    }
}