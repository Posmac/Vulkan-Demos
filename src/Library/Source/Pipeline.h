#pragma once

#include "Library/Core/Core.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandBuffer.h"
#include "Resources.h"
#include "DescriptorSets.h"
#include "Library/Structs/Pipeline.h"
#include "Library/Common/Tools.h"

namespace vk
{
    void CreateShaderModule(VkDevice device,
        std::vector<unsigned char>& sourceCode,
        VkShaderModule& shaderModule);

    void SpecifyPipelineShaderStages(std::vector<ShaderStageParams>& shaderStageParams,
        std::vector<VkPipelineShaderStageCreateInfo>& shaderStageInfo);

    void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription>& bindDescriptions,
        std::vector<VkVertexInputAttributeDescription>& attributes,
        VkPipelineVertexInputStateCreateInfo& stateInfo);

    void SpecifyPipelineInputAssemblyStage(VkPrimitiveTopology topology,
        bool primitiveRestartEnable,
        VkPipelineInputAssemblyStateCreateInfo& stateInfo);

    void SpecifyPipelineRasterizationState(bool depthClampEnable,
        bool rasterizedDiscardEnable,
        VkPolygonMode polygonMode,
        VkCullModeFlags cullmode,
        VkFrontFace frontFace,
        bool depthBiasEnable,
        float biasConstant,
        float biasClamp,
        float biasSlope,
        float lineWidth,
        VkPipelineRasterizationStateCreateInfo& stateInfo);

    void SpecifyPipelineTesselationState(uint32_t pathControlPointsCount,
        VkPipelineTessellationStateCreateInfo& stateInfo);

    void SpecifyPipelineViewportAndScissorTestState(ViewportInfo& viewportInfo,
        VkPipelineViewportStateCreateInfo& stateInfo);

    void SpecifyPipelineMultisamlpeState(VkSampleCountFlagBits sampleCount,
        bool perSampleShadingEnable,
        float minSampleShading,
        VkSampleMask *sampleMask,
        bool alphaToCoverageEnable,
        bool alphaToOneEnable,
        VkPipelineMultisampleStateCreateInfo& stateInfo);

    void SpecifPipelineDepthAndStencilState(bool depthTestEnable,
        bool depthWriteEnable,
        VkCompareOp depthCompareOp,
        bool depthBoundsTestEnable,
        float minDepthBounds,
        float maxDepthBounds,
        bool stencilTestEnable,
        VkStencilOpState frontStencilTestParams,
        VkStencilOpState backStencilTestParams,
        VkPipelineDepthStencilStateCreateInfo& stateInfo);

    void SpecifyPipelineBlendState(bool logicOpEnable,
        VkLogicOp logicOp,
        std::vector<VkPipelineColorBlendAttachmentState>& attachmentBlendStates,
        const std::array<float, 4>& blendConstants,
        VkPipelineColorBlendStateCreateInfo& stateInfo);

    void SpecifyPipelineDynamicStates(std::vector<VkDynamicState>& dynamicStates,
        VkPipelineDynamicStateCreateInfo& stateInfo);

    void CreatePipelineLayout(VkDevice device,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkPushConstantRange>& pushConstantsRange,
        VkPipelineLayout& pipelineLayout);

    void SpecifyGraphicsPipelineParameters(VkPipelineCreateFlags additionalOptions,
        std::vector<VkPipelineShaderStageCreateInfo>& shaderStageInfo,
        VkPipelineVertexInputStateCreateInfo& vertexInputStateCreateInfo,
        VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo,
        VkPipelineTessellationStateCreateInfo* tessellationStateCreateInfo,
        VkPipelineViewportStateCreateInfo* viewportStateCreateInfo,
        VkPipelineRasterizationStateCreateInfo& rasterizationStateCreateInfo,
        VkPipelineMultisampleStateCreateInfo* multisampleStateCreateInfo,
        VkPipelineDepthStencilStateCreateInfo* depthAndStencilStateCreateInfo,
        VkPipelineColorBlendStateCreateInfo* blendStateCreateInfo,
        VkPipelineDynamicStateCreateInfo* dynamicStateCreatInfo,
        VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass,
        uint32_t subpass,
        VkPipeline basePipelineHandle,
        int32_t basepipelineIndex,
        VkGraphicsPipelineCreateInfo& graphicsPipelineCreateinfo);

    void CreatePipelineCacheObject(VkDevice device,
        std::vector<unsigned char*>& cacheData,
        VkPipelineCache& cache);

    bool RetrieveDataFromPipelineCache(VkDevice device,
        VkPipelineCache pipelineCache,
        std::vector<unsigned char>& data);

    void MergeMultiplePipelineCacheObjects(VkDevice device,
        VkPipelineCache targetCache,
        std::vector<VkPipelineCache>& sourceCaches);

    void CreateGraphicsPipelines(VkDevice device,
        const std::vector<VkGraphicsPipelineCreateInfo>& graphicsPipelinesInfo,
        VkPipelineCache cache,
        std::vector<VkPipeline>& pipelines);

    void CreateComputePipeline(VkDevice device,
        VkPipelineCreateFlags additionalOptions,
        VkPipelineShaderStageCreateInfo& computeShaderStage,
        VkPipelineLayout pipelineLayout,
        VkPipeline basePipeline,
        VkPipelineCache cache,
        VkPipeline& computePipeline);

    void BindPipelineObject(VkCommandBuffer commandBuffer,
        VkPipelineBindPoint type,
        VkPipeline pipeline);

    void CreatePipelineLayoutWithCombinedImageSamplerBufferAndPushConstants(VkDevice device,
        std::vector<VkPushConstantRange>& range,
        VkDescriptorSetLayout& descritorLayout,
        VkPipelineLayout& pipelineLayout);

    bool CreateGraphicsPipelineWithVertexAndFragmntShadersDepthTestEnableAndWithDynamicViewportAndScissorsTest(VkDevice device,
        VkPipelineCreateFlags additionalOptions,
        std::string& vertexShaderFile,
        std::string& fragmentShaderFile,
        std::vector<VkVertexInputBindingDescription>& vertexInputBindingDescriptions,
        std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions,
        VkPrimitiveTopology primitiveTopology,
        bool primitiveRestartEnable,
        VkPolygonMode polygonMode,
        VkCullModeFlags cullMode,
        VkFrontFace frontFace,
        bool logicOpEnabled,
        VkLogicOp logicOp,
        std::vector<VkPipelineColorBlendAttachmentState>& attachmentsBlendStates,
        std::array<float, 4>& blendContants,
        VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass,
        uint32_t subpass,
        VkPipeline basePipeline,
        VkPipelineCache pipelineCache,
        std::vector<VkPipeline>& graphicsPipeline);

    bool CreateMultipleGraphicsPipelinesOnMultipleThreads(VkDevice device,
        std::string& pipelineCacheFile,
        std::vector<std::vector<VkGraphicsPipelineCreateInfo>>& pipelinesInfos,
        std::vector<std::vector<VkPipeline>>& graphicsPipelines);

    void DestroyPipeline(VkDevice device,
        VkPipeline& pipeline);

    void DestroyPipelineCache(VkDevice device,
        VkPipelineCache& pipelineCache);

    void DestroyPipelineLayout(VkDevice device,
        VkPipelineLayout& pipelineLayout);

    void DestroyShaderModule(VkDevice device,
        VkShaderModule& module);

}