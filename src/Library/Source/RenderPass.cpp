#include "RenderPass.h"

namespace vk
{
    void SpecifySubpassDescriptions(std::vector<SubpassParams>& subpassParams,
        std::vector<VkSubpassDescription>& subpassDescriptions)
    {
        subpassDescriptions.clear();

        for (auto& subpassParam : subpassParams)
        {
            subpassDescriptions.push_back(
                {
                    0,
                    subpassParam.pipelineType,
                    static_cast<uint32_t>(subpassParam.inputAttachments.size()),
                    subpassParam.inputAttachments.data(),
                    static_cast<uint32_t>(subpassParam.colorAttachments.size()),
                    subpassParam.colorAttachments.data(),
                    subpassParam.resolveAttachments.data(),
                    &subpassParam.depthStencilAttachment,
                    static_cast<uint32_t>(subpassParam.preserveAttachments.size()),
                    subpassParam.preserveAttachments.data()
                }
            );
        }
    }

    void CreateRenderPass(VkDevice device,
        std::vector<VkAttachmentDescription>& attachments,
        std::vector<SubpassParams>& subpassParams,
        std::vector<VkSubpassDependency>& subpassDependencies,
        VkRenderPass& renderPass)
    {
        std::vector<VkSubpassDescription> subpassDescriptions;
        SpecifySubpassDescriptions(subpassParams, subpassDescriptions);

        VkRenderPassCreateInfo renderPassInfo =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(attachments.size()),
            attachments.data(),
            static_cast<uint32_t>(subpassDescriptions.size()),
            subpassDescriptions.data(),
            static_cast<uint32_t>(subpassDependencies.size()),
            subpassDependencies.data(),
        };

        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    }

    void CreateFramebuffer(VkDevice device,
        VkRenderPass renderPass,
        const std::vector<VkImageView>& attachments,
        uint32_t width,
        uint32_t height,
        uint32_t layers,
        VkFramebuffer& frameBuffer)
    {
        VkFramebufferCreateInfo framebufferInfo =
        {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            0,
            renderPass,
            static_cast<uint32_t>(attachments.size()),
            attachments.data(),
            width,
            height,
            layers
        };

        VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffer));
    }

    void PrepareRenderPassForGeometryRenderingAndPostProcessSubpasses(VkDevice device,
        VkRenderPass renderPass)
    {
        std::vector<VkAttachmentDescription> attachmentsDescriptions = {
            {
                0,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            {
                0,
                VK_FORMAT_D16_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
            {
                0,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
        };

        VkAttachmentReference depth_stencil_attachment = 
        {
          1,
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        SubpassParams params =
        {
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            {},
            {
                {
                    0,
                    VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
                }
            },
            {},
            depth_stencil_attachment,
            {},
        };

        std::vector<SubpassParams> subpassParams = 
        {
            {
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                {},
                {
                    {
                        0,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    }
                },
                {},
                depth_stencil_attachment,
                {},
            },

            {
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                {
                    {
                        0,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    }
                },
                {
                    {
                        2,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    }
                },
                {},
                {},
                {},
            }
        };

        std::vector<VkSubpassDependency> subpassDependencies = {
            {
                0,
                1,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                VK_DEPENDENCY_BY_REGION_BIT
            }
        };

        CreateRenderPass(device, attachmentsDescriptions, subpassParams, subpassDependencies, renderPass);
    }

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
        VkFramebuffer& framebuffer)
    {
        Create2DImageAndView(device, gpu, VK_FORMAT_R8G8B8A8_UNORM, { width, height }, 1, 1, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, colorImage,
            colorImageMemory, colorImageView);

        Create2DImageAndView(device, gpu, VK_FORMAT_D16_UNORM, { width, height }, 1, 1, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, depthImage,
            depthImageMemory, depthImageView);

        std::vector<VkAttachmentDescription> attachemtDescriptions =
        {
            {
                0,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
            {
                0,
                VK_FORMAT_D16_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            },
        };

        VkAttachmentReference depthStencilAttachment = 
        {
            1,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        std::vector<SubpassParams> subpassParams =
        {
            {
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                {},
                {
                    {
                        0,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    }
                },
                {},
                depthStencilAttachment,
                {}
            },
        };

        std::vector<VkSubpassDependency> subpassDependencies =
        {
            {
                0,
                VK_SUBPASS_EXTERNAL,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                0
            },
        };

        CreateRenderPass(device, attachemtDescriptions, subpassParams, subpassDependencies, renderPass);
        CreateFramebuffer(device, renderPass, { colorImageView, depthImageView }, width, height, 1, framebuffer);
    }

    void BeginRenderPass(VkCommandBuffer commandBuffer, 
        VkRenderPass renderPass, 
        VkFramebuffer framebuffer, 
        VkRect2D renderArea, 
        const std::vector<VkClearValue>& clearValues, 
        VkSubpassContents subpassContents)
    {
        VkRenderPassBeginInfo beginInfo =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            renderPass,
            framebuffer,
            renderArea,
            static_cast<uint32_t>(clearValues.size()),
            clearValues.data()
        };

        vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
    }

    void ProgressToTheNextSubpass(VkCommandBuffer commandBuffer, 
        VkSubpassContents subpassContents)
    {
        vkCmdNextSubpass(commandBuffer, subpassContents);
    }

    void EndRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    void DestroyFramebuffer(VkDevice device, 
        VkFramebuffer& framebuffer)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    void DestroyRenderPass(VkDevice device, 
        VkRenderPass& renderPass)
    {
        vkDestroyRenderPass(device, renderPass, nullptr);
    }
}