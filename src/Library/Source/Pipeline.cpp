#include "Pipeline.h"

namespace vk
{
    void CreateShaderModule(VkDevice device, 
        std::vector<unsigned char>& sourceCode, 
        VkShaderModule& shaderModule)
    {
        VkShaderModuleCreateInfo moduleInfo =
        {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            nullptr,
            0, 
            sourceCode.size(),
            reinterpret_cast<uint32_t const *>(sourceCode.data())
        };

        VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleInfo, nullptr, &shaderModule));
    }

    void SpecifyPipelineShaderStages(std::vector<ShaderStageParams>& shaderStageParams, 
        std::vector<VkPipelineShaderStageCreateInfo>& shaderStageInfo)
    {
        shaderStageInfo.clear();
        for (auto& param : shaderStageParams)
        {
            shaderStageInfo.push_back(
                {
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    nullptr,
                    0,
                    param.shaderStage,
                    param.shaderModule,
                    param.entryPoint,
                    param.specialisationInfo
                });
        }
    }

    void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription>& bindDescriptions, 
        std::vector<VkVertexInputAttributeDescription>& attributes, 
        VkPipelineVertexInputStateCreateInfo& stateInfo)
    {
        stateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(bindDescriptions.size()),
            bindDescriptions.data(),
            static_cast<uint32_t>(attributes.size()),
            attributes.data(),
        };
    }

    void SpecifyPipelineInputAssemblyStage(VkPrimitiveTopology topology, 
        bool primitiveRestartEnable, 
        VkPipelineInputAssemblyStateCreateInfo& stateInfo)
    {
        stateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            nullptr, 
            0,
            topology,
            primitiveRestartEnable
        };
    }

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
        VkPipelineRasterizationStateCreateInfo& stateInfo)
    {
        stateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            nullptr,
            0,
            depthClampEnable,
            rasterizedDiscardEnable,
            polygonMode,
            cullmode,
            frontFace,
            depthBiasEnable,
            biasConstant,
            biasClamp,
            biasSlope,
            lineWidth
        };
    }

    void SpecifyPipelineTesselationState(uint32_t pathControlPointsCount, 
        VkPipelineTessellationStateCreateInfo& stateInfo)
    {
        stateInfo = 
        {
            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            nullptr,
            0, pathControlPointsCount
        };
    }

    void SpecifyPipelineViewportAndScissorTestState(ViewportInfo& viewportInfo, 
        VkPipelineViewportStateCreateInfo& stateInfo)
    {
        stateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(viewportInfo.viewPorts.size()),
            viewportInfo.viewPorts.data(),
            static_cast<uint32_t>(viewportInfo.scissors.size()),
            viewportInfo.scissors.data(),
        };
    }

    void SpecifyPipelineMultisamlpeState(VkSampleCountFlagBits sampleCount, 
        bool perSampleShadingEnable, 
        float minSampleShading, 
        VkSampleMask sampleMask, 
        bool alphaToCoverageEnable, 
        bool alphaToOneEnable, 
        VkPipelineMultisampleStateCreateInfo& stateInfo)
    {
        stateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            nullptr,
            0,
            sampleCount, 
            perSampleShadingEnable,
            minSampleShading,
            &sampleMask,
            alphaToCoverageEnable,
            alphaToOneEnable
        };
    }

    void SpecifPipelineDepthAndStencilState(bool depthTestEnable, 
        bool depthWriteEnable, 
        VkCompareOp depthCompareOp, 
        bool depthBoundsTestEnable, 
        float minDepthBounds, 
        float maxDepthBounds, 
        bool stencilTestEnable, 
        VkStencilOpState frontStencilTestParams, 
        VkStencilOpState backStencilTestParams, 
        VkPipelineDepthStencilStateCreateInfo& stateInfo)
    {
        stateInfo = 
        {
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            nullptr,
            0,
            depthTestEnable,
            depthWriteEnable,
            depthCompareOp,
            depthBoundsTestEnable,
            stencilTestEnable,
            frontStencilTestParams,
            backStencilTestParams,
            minDepthBounds,
            maxDepthBounds
        };
    }

    void SpecifyPipelineBlendState(bool logicOpEnable, 
        VkLogicOp logicOp, 
        std::vector<VkPipelineColorBlendAttachmentState>& attachmentBlendStates, 
        const std::array<float, 4>& blendConstants, 
        VkPipelineColorBlendStateCreateInfo& stateInfo)
    {
        stateInfo = 
        {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            nullptr,
            0,
            logicOpEnable,
            logicOp,
            static_cast<uint32_t>(attachmentBlendStates.size()),
            attachmentBlendStates.data(),
            {
                blendConstants[0],
                blendConstants[1],
                blendConstants[2],
                blendConstants[3],
            }
        };
    }

    void SpecifyPipelineDynamicStates(std::vector<VkDynamicState>& dynamicStates, 
        VkPipelineDynamicStateCreateInfo& stateInfo)
    {
        stateInfo = 
        {
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(dynamicStates.size()),
            dynamicStates.data()
        };
    }

    void CreatePipelineLayout(VkDevice device, 
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, 
        const std::vector<VkPushConstantRange>& pushConstantsRange, 
        VkPipelineLayout& pipelineLayout)
    {
        VkPipelineLayoutCreateInfo layoutInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(descriptorSetLayouts.size()),
            descriptorSetLayouts.data(),
            static_cast<uint32_t>(pushConstantsRange.size()),
            pushConstantsRange.data()
        };

        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout));
    }

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
        VkGraphicsPipelineCreateInfo& graphicsPipelineCreateinfo)
    {
        graphicsPipelineCreateinfo = 
        {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            nullptr,
            additionalOptions,
            static_cast<uint32_t>(shaderStageInfo.size()),
            shaderStageInfo.data(),
            &vertexInputStateCreateInfo,
            &inputAssemblyStateCreateInfo,
            tessellationStateCreateInfo,
            viewportStateCreateInfo,
            &rasterizationStateCreateInfo,
            multisampleStateCreateInfo,
            depthAndStencilStateCreateInfo,
            blendStateCreateInfo,
            dynamicStateCreatInfo,
            pipelineLayout,
            renderPass,
            subpass,
            basePipelineHandle,
            basepipelineIndex
        };
    }

    void CreatePipelineCacheObject(VkDevice device, 
        std::vector<unsigned char>& cacheData, 
        VkPipelineCache& cache)
    {
        VkPipelineCacheCreateInfo cacheInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(cacheData.size()),
            cacheData.data()
        };

        VK_CHECK_RESULT(vkCreatePipelineCache(device, &cacheInfo, nullptr, &cache));
    }

    bool RetrieveDataFromPipelineCache(VkDevice device, 
        VkPipelineCache pipelineCache, 
        std::vector<unsigned char>& data)
    {
        size_t dataSize = 0;
        VK_CHECK_RESULT(vkGetPipelineCacheData(device, pipelineCache, &dataSize, nullptr));

        if (dataSize == 0)
        {
            WARN_LOG("Could not get the size of pipeline cache");
            return false;
        }

        data.resize(dataSize);

        VK_CHECK_RESULT(vkGetPipelineCacheData(device, pipelineCache, &dataSize, data.data()));

        if (data.size() == 0)
        {
            WARN_LOG("Could not acquire pipeline cache data");
            return false;
        }

        return true;
    }

    void MergeMultiplePipelineCacheObjects(VkDevice device, 
        VkPipelineCache targetCache, 
        std::vector<VkPipelineCache>& sourceCaches)
    {
        VK_CHECK_RESULT(vkMergePipelineCaches(device, targetCache, static_cast<uint32_t>(sourceCaches.size()),
            sourceCaches.data()));
    }

    void CreateGraphicsPipelines(VkDevice device, 
        const std::vector<VkGraphicsPipelineCreateInfo>& graphicsPipelinesInfo, 
        VkPipelineCache cache, 
        std::vector<VkPipeline>& pipelines)
    {
        pipelines.resize(graphicsPipelinesInfo.size());
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, cache, static_cast<uint32_t>(graphicsPipelinesInfo.size()),
            graphicsPipelinesInfo.data(), nullptr, pipelines.data()));
    }

    void CreateComputePipeline(VkDevice device, 
        VkPipelineCreateFlags additionalOptions, 
        VkPipelineShaderStageCreateInfo& computeShaderStage, 
        VkPipelineLayout pipelineLayout, 
        VkPipeline basePipeline, 
        VkPipelineCache cache, 
        VkPipeline& computePipeline)
    {
        VkComputePipelineCreateInfo info =
        {
            VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            nullptr,
            additionalOptions,
            computeShaderStage,
            pipelineLayout,
            basePipeline,
            -1
        };

        VK_CHECK_RESULT(vkCreateComputePipelines(device, cache, 1, &info, nullptr, &computePipeline));
    }

    void BindPipelineObject(VkCommandBuffer commandBuffer, 
        VkPipelineBindPoint type, 
        VkPipeline pipeline)
    {
        vkCmdBindPipeline(commandBuffer, type, pipeline);
    }

    void CreatePipelineLayoutWithCombinedImageSamplerBufferAndPushConstants(VkDevice device, 
        std::vector<VkPushConstantRange>& range, 
        VkDescriptorSetLayout& descritorLayout, 
        VkPipelineLayout& pipelineLayout)
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetBindings =
        {
            {
                0,
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                1, 
                VK_SHADER_STAGE_FRAGMENT_BIT,
                nullptr
            },
            {
                1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1, 
                VK_SHADER_STAGE_FRAGMENT_BIT,
                nullptr
            },
        };
        CreateDescriptorSetLayout(device, descriptorSetBindings, descritorLayout);
        CreatePipelineLayout(device, { descritorLayout }, range, pipelineLayout);
    }

    bool CreateGraphicsPipelineWithVertexAndFragmentShadersDepthTestEnableAndWithDynamicViewportAndScissorsTest(VkDevice device, 
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
        std::vector<VkPipeline>& graphicsPipeline)
    {
        std::vector<unsigned char> vertexShaderSpirv;
        if (!GetBinaryFileContents(vertexShaderFile, vertexShaderSpirv))
        {
            return false;
        }

        VkShaderModule vertexShaderModule;
        CreateShaderModule(device, vertexShaderSpirv, vertexShaderModule);

        std::vector<unsigned char> fragmeShaderSpirv;
        if (!GetBinaryFileContents(fragmentShaderFile, fragmeShaderSpirv))
        {
            return false;
        }

        VkShaderModule fragmentShaderModule;
        CreateShaderModule(device, fragmeShaderSpirv, fragmentShaderModule);

        std::vector<ShaderStageParams> shaderStageParams = 
        {
            {
                VK_SHADER_STAGE_VERTEX_BIT,
                vertexShaderModule,
                "main",
                nullptr,
            },
            {
                VK_SHADER_STAGE_FRAGMENT_BIT,
                fragmentShaderModule,
                "main",
                nullptr,
            },
        };

        std::vector<VkPipelineShaderStageCreateInfo> shaderStagesInfo;
        SpecifyPipelineShaderStages(shaderStageParams, shaderStagesInfo);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo;
        SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, 
            vertexAttributeDescriptions, vertexInputInfo);

        VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
        SpecifyPipelineInputAssemblyStage(primitiveTopology, primitiveRestartEnable, assemblyInfo);

        VkViewport viewport = {
            0.0f, 
            0.0f,
            500.0f, //magic nums
            500.0f, //magic nums
            0.0f,
            1.0f,
        };

        VkOffset2D offset = { 0, 0 };
        VkExtent2D extent = { 500, 500 };
        VkRect2D rect = { offset, extent };

        ViewportInfo viewportInfo = 
        {
            {viewport}, {rect}
        };

        VkPipelineViewportStateCreateInfo viewportStateInfo;
        SpecifyPipelineViewportAndScissorTestState(viewportInfo, viewportStateInfo);

        VkPipelineRasterizationStateCreateInfo resterizationState;
        SpecifyPipelineRasterizationState(false, false, polygonMode, cullMode, frontFace, 
            false, 0.0f, 1.0f, 0.0f, 1.0f, resterizationState);

        VkPipelineMultisampleStateCreateInfo msInfo;
        SpecifyPipelineMultisamlpeState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, 0, false, false,
            msInfo);

        VkStencilOpState stencilTestState =
        {
            VK_STENCIL_OP_KEEP,
            VK_STENCIL_OP_KEEP,
            VK_STENCIL_OP_KEEP,
            VK_COMPARE_OP_ALWAYS,
            0,
            0,
            0
        };

        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        SpecifPipelineDepthAndStencilState(true, true, VK_COMPARE_OP_LESS_OR_EQUAL,
            false, 0.0f, 1.0f, false, stencilTestState, stencilTestState, depthStencilInfo);

        VkPipelineColorBlendStateCreateInfo blendInfo;
        SpecifyPipelineBlendState(logicOpEnabled, logicOp, attachmentsBlendStates, blendContants,
            blendInfo);

        std::vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicState;
        SpecifyPipelineDynamicStates(dynamicStates, dynamicState);

        VkGraphicsPipelineCreateInfo pipelineInfo;
        SpecifyGraphicsPipelineParameters(additionalOptions, shaderStagesInfo, vertexInputInfo,
            assemblyInfo, nullptr, &viewportStateInfo, resterizationState, &msInfo, &depthStencilInfo,
            &blendInfo, &dynamicState, pipelineLayout, renderPass, subpass, basePipeline, -1,
            pipelineInfo);

        CreateGraphicsPipelines(device, { pipelineInfo }, pipelineCache, graphicsPipeline);
        return true;
    }

    bool CreateMultipleGraphicsPipelinesOnMultipleThreads(VkDevice device, 
        std::string& pipelineCacheFile, 
        std::vector<std::vector<VkGraphicsPipelineCreateInfo>>& pipelinesInfos, 
        std::vector<std::vector<VkPipeline>>& graphicsPipelines)
    {
        std::vector<unsigned char> cacheData;
        if (!GetBinaryFileContents(pipelineCacheFile, cacheData))
        {
            return false;
        }

        std::vector<VkPipelineCache> caches(pipelinesInfos.size());
        for (size_t i = 0; i < pipelinesInfos.size(); i++)
        {
            CreatePipelineCacheObject(device, cacheData, caches[i]);
        }

        std::vector<std::thread>threads (pipelinesInfos.size());
        for (size_t i = 0; i < pipelinesInfos.size(); i++)
        {
            graphicsPipelines[i].resize(pipelinesInfos[i].size());
            threads[i] = std::thread(CreateGraphicsPipelines, device, std::ref(pipelinesInfos[i]), caches[i],
                std::ref(graphicsPipelines[i]));
        }

        for (size_t i = 0; i < pipelinesInfos.size(); i++)
        {
            threads[i].join();
        }

        VkPipelineCache targetCache = caches.back();
        std::vector<VkPipelineCache> srcCaches(caches.size() - 1);
        for (size_t i = 0; i < caches.size(); i++)
        {
            srcCaches[i] = caches[i];
        }

        MergeMultiplePipelineCacheObjects(device, targetCache, srcCaches);
        RetrieveDataFromPipelineCache(device, targetCache, cacheData);

        return true;
    }

    void DestroyPipeline(VkDevice device, 
        VkPipeline& pipeline)
    {
        vkDestroyPipeline(device, pipeline, nullptr);
    }

    void DestroyPipelineCache(VkDevice device, 
        VkPipelineCache& pipelineCache)
    {
        vkDestroyPipelineCache(device, pipelineCache, nullptr);
    }

    void DestroyPipelineLayout(VkDevice device, 
        VkPipelineLayout& pipelineLayout)
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void DestroyShaderModule(VkDevice device,
        VkShaderModule& module)
    {
        vkDestroyShaderModule(device, module, nullptr);
    }
}


