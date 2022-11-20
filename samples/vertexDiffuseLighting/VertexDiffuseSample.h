#pragma once

#include "VulkanSampleBase.h"

namespace vk
{
    class VertexDiffuseSample : public VulkanSample
    {
    public:
        virtual bool Initialize(WindowParameters& windowParams,
            std::vector<const char*> validationLayer,
            std::vector<const char*> instanceExtensions,
            std::vector<const char*> deviceExtensions) override;
        virtual bool Draw()  override;
        virtual bool Resize()  override;
        virtual void Destroy()  override;

    private:
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
        UniformBufferObject uniformObject;
    };
}