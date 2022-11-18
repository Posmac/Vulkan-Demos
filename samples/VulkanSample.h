#pragma once

#include "VulkanSampleBase.h"

namespace vk
{
    class Sample : public SampleBase
    {
    public:
        bool InitVulkan(WindowParameters& windowParams,
            std::vector<const char*> validationLayer,
            std::vector<const char*> instanceExtensions,
            std::vector<const char*> deviceExtensions,
            VkPhysicalDeviceFeatures* deviceFeatures = nullptr,
            bool useDepth = true,
            VkImageUsageFlags swapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        bool CreateSwapchain(bool useDepth = true,
            VkImageUsageFlags swapChainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VkImageUsageFlags depthImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

        virtual bool Initialize(WindowParameters& windowParams,
            std::vector<const char*> validationLayer,
            std::vector<const char*> instanceExtensions,
            std::vector<const char*> deviceExtensions) override;
        virtual bool Draw()  override;
        virtual bool Resize()  override;
        virtual void Destroy()  override;

        bool RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkFramebuffer framebuffer);

    private:
        //base parameters
        VkDebugUtilsMessengerEXT messenger;
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkSurfaceKHR presentationSurface;
        QueueParameters graphicsQueue;
        QueueParameters presentQueue;
        QueueParameters computeQueue;
        SwapchainParameters swapchain;
        VkCommandPool commandPool;
        std::vector<VkImage> depthImages;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkDeviceMemory> depthImageMemory;
        std::vector<FrameResources> frameResources;
        uint32_t framesCount = 3;
        VkFormat depthFormat = VK_FORMAT_D16_UNORM;

        //additional
        Mesh model;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        bool updateUniformBuffer;
        VkBuffer uniformBuffer;
        VkDeviceMemory UniformBufferMemory;
    };
}