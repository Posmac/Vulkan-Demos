#pragma once

#include "Library/Core/Core.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandBuffer.h"
#include "Resources.h"
#include "Library/Structs/Buffer.h"
#include "Library/Structs/Image.h"
#include "Library/Structs/Descriptors.h"
#include "Library/Structs/RenderPass.h"

namespace vk
{
    void SpecifySubpassDescriptions(std::vector<SubpassParams>& subpassParams,
        std::vector<VkSubpassDescription>& subpassDescrittions);

    void CreateRenderPass(VkDevice device,
        std::vector<VkAttachmentDescription>& attachments,
        std::vector<SubpassParams>& subpassParams,
        std::vector<VkSubpassDependency>& subpassDependencies,
        VkRenderPass& renderPass);

    void CreateFramebuffer(VkDevice device,
        VkRenderPass renderPass,
        const std::vector<VkImageView>& attachments,
        uint32_t width,
        uint32_t height,
        uint32_t layers,
        VkFramebuffer& frameBuffer);

    void PrepareRenderPassForGeometryRenderingAndPostProcessSubpasses(VkDevice device,
        VkRenderPass renderPass);

    void PrepareREnderPassAndFramebufferWithColorAndDepthAttachment(VkDevice device,
        VkPhysicalDevice gpu,
        uint32_t width,
        uint32_t height,
        VkImage& colorImage,
        VkImageView& colorImageView,
        VkDeviceMemory& colorImageMemory,
        VkImage& depthImage,
        VkImageView& depthImageView,
        VkDeviceMemory& depthImageMemory,
        VkRenderPass& renderPass,
        VkFramebuffer& framebuffer);

    void BeginRenderPass(VkCommandBuffer commandBuffer,
        VkRenderPass renderPass,
        VkFramebuffer framebuffer,
        VkRect2D renderArea,
        std::vector<VkClearValue>& clearValues,
        VkSubpassContents subpassContents);

    void ProgressToTheNextSubpass(VkCommandBuffer commandBuffer,
        VkSubpassContents subpassContents);

    void EndRenderPass(VkCommandBuffer commandBuffer);

    void DestroyFramebuffer(VkDevice device,
        VkFramebuffer& framebuffer);

    void DestroyRenderPass(VkDevice device,
        VkRenderPass& renderPass);
}