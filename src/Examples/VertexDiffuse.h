#pragma once

#include "vk/VulkanEntry.h"

namespace vk
{
    class VertexDiffuseExample
    {
    public:
        VertexDiffuseExample();
        VertexDiffuseExample(const std::vector<const char*>& layers,
            const std::vector<const char*>& instanceExtensions,
            const std::vector<const char*>& deviceExtensions,
            bool isDebugModeEnabled,
            int width,
            int height,
            WindowParameters& params);

        void EnumerateAvailableLayerProperties(uint32_t& availableLayersCount,
            std::vector<VkLayerProperties>& availableLayerProperties,
            bool isDebugModeEnabled);

        void AcquireSwapchainImage(VkDevice device,
            VkSwapchainKHR swapchain,
            VkSemaphore semaphore,
            VkFence fence,
            uint32_t& index);

        void PrepareAnimationFrame(VkDevice device,
            VkQueue graphicsQueue,
            VkQueue presentQueue,
            VkSwapchainKHR swapchain,
            VkExtent2D swapchainSize,
            std::vector<VkImageView>& swapChainViews,
            VkImageView depthAttachment,
            std::vector<WaitSemaphoreInfo> wainInfo,
            FrameResources& frameResource,
            VkRenderPass renderPass);

        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,
            VkFramebuffer framebuffer);

        void PresentImage(VkQueue queue, std::vector<VkSemaphore> renderingSemaphores,
            std::vector<PresentInfo> imagesToPresent);

        void Draw();

    private:
        VulkanEntry entry;
        DebugMessenger debugMessenger;
        Instance instance;
        LogicalDevice device;
        PhysicalDevice gpu;
        CommandPool commandPool;
        CommandBuffer commandBuffer;

        Swapchain swapchain;
        std::vector<VkImageView> swapChainImageViews;
        VkRect2D scirssor;
        QueuesInfo queueInfo;
        VkImage depthImage;
        VkImageView depthImageView;
        VkDeviceMemory depthImagesMemory;
        FrameResources framesResources;

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
        VkDeviceMemory uniformBufferMemory;
    };
}