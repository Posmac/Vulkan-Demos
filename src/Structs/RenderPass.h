#pragma once

#include <vector>
#include "vulkan/vulkan.h"

namespace vk
{
    struct SubpassParams
    {
        VkPipelineBindPoint pipelineType;
        std::vector<VkAttachmentReference> inputAttachments;
        std::vector<VkAttachmentReference> colorAttachments;
        std::vector<VkAttachmentReference> resolveAttachments;
        VkAttachmentReference depthStencilAttachment;
        std::vector<uint32_t> preserveAttachments;
    };

    struct FrameResources 
    {
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAcquiredSemaphore;
        VkSemaphore readyToPresentSemaphore;
        VkFence drawingFinishedFence;
        VkImageView depthAttachment;
        VkFramebuffer framebuffer;
    };
}