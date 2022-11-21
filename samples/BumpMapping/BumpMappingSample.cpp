#include "BumpMapping/BumpMappingSample.h"

namespace vk
{
    bool BumpMappingSample::Initialize(WindowParameters& windowParams,
        std::vector<const char*> validationLayer,
        std::vector<const char*> instanceExtensions,
        std::vector<const char*> deviceExtensions)
    {
        if (!InitVulkan(windowParams, validationLayer, instanceExtensions, deviceExtensions, nullptr,
            false, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
            return false;
        }

        //load texture data
        std::vector<unsigned char> imageData;
        int imageW, imageH, nrChannels, dataSize;
        if (!LoadTextureDataFromFile("textures/normal_map.png", 4, imageData, &imageW, &imageH, &nrChannels, &dataSize))
        {
            return false;
        }
  
        if (!CreateCombinedImageSampler(device, physicalDevice, VK_IMAGE_TYPE_2D, swapchain.format,
            { (uint32_t)imageW, (uint32_t)imageH, 1 }, 1, 1, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            false, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, VK_FILTER_NEAREST, VK_FILTER_NEAREST,
            VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
            0.0f, false, 0.0f, false, VK_COMPARE_OP_ALWAYS, 0.0, 0.0, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, false,
            normalSampler, normalTexture, normalTextureView, normalTextureMemory))
        {
            return false;
        }

        VkImageSubresourceLayers subresourceLayer =
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, 
            0,
            1
        };

        UseStaginBufferToUpdateImageWithDeviceLocalMemoryBound(device, physicalDevice, static_cast<VkDeviceSize>(imageData.size()),
            &imageData[0], normalTexture, subresourceLayer, { 0,0,0 }, { (uint32_t)imageW, (uint32_t)imageH, 1 },
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            graphicsQueue.handle, frameResources.front().commandBuffer, {});
   
        //load mesh data to memory
        MeshLoader::LoadMesh("models/ice.obj", true, true, true, true, model);
        VkDeviceSize vertexBufferSize = sizeof(model.data[0]) * model.data.size();
        CreateBuffer(device, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            vertexBuffer);
        AllocateAndBindMemoryObjectToBuffer(device, physicalDevice, vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertexBufferMemory);

        UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(device, physicalDevice, vertexBufferSize,
            &model.data[0], vertexBuffer, 0, 0, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            graphicsQueue.handle, frameResources.front().commandBuffer, {});

        //load matrix data throught staging buffer into uniform buffer
        VkDeviceSize uniformBufferSize = sizeof(UniformBufferObject);
        CreateBuffer(device, uniformBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
        AllocateAndBindMemoryObjectToBuffer(device, physicalDevice, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            stagingBufferMemory);

        CreateUniformBuffer(device, physicalDevice, uniformBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffer, uniformBufferMemory);

        updateUniformBuffer = true;
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), 
            glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f),
            static_cast<float>(swapchain.size.width) / static_cast<float>(swapchain.size.height),
            0.01f, 100.0f);
        projectionMatrix[1][1] *= -1;

          uniformObject = {
            modelMatrix,
            viewMatrix,
            projectionMatrix
        };

        MapUpdateAndUnmapHostVisibleMemory(device, stagingBufferMemory, 0, uniformBufferSize,
            &uniformObject, true, nullptr);

        //descriptor set with uniform buffer
        std::vector<VkDescriptorSetLayoutBinding> descriptorLayoutBindings =
        {
            {
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                VK_SHADER_STAGE_VERTEX_BIT,
                nullptr
            },
            {
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                nullptr
            }
        };

        CreateDescriptorSetLayout(device, descriptorLayoutBindings, descriptorSetLayout);

        std::vector<VkDescriptorPoolSize> descriptorPoolSize =
        {
            {  
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1
            },
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1
            },
        };

        CreateDescriptorPool(device, false, 1, descriptorPoolSize, descriptorPool);
        AllocateDescriptorSets(device, descriptorPool, { descriptorSetLayout }, descriptorSets);

        VkDescriptorBufferInfo bufferInfo =
        {
            uniformBuffer,
            0,
            VK_WHOLE_SIZE
        };

        BufferDescriptorInfo bufferDescriptorUpdate =
        {
            descriptorSets[0],
            0,
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            { bufferInfo }
        };

        VkDescriptorImageInfo imageInfo =
        {
            normalSampler,
            normalTextureView, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        ImageDescriptorInfo imageDescriptorInfo =
        {
            descriptorSets[0],
            1, 
            0,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            {imageInfo}
        };

        UpdateDescriptorSets(device, { imageDescriptorInfo }, { bufferDescriptorUpdate }, {}, {});

        //RenderPass
        std::vector<VkAttachmentDescription> attachmentDescriptions =
        {
            {
                0,
                swapchain.format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            },
            {
                0,
                depthFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            },
        };

        VkAttachmentReference depthAttachment = 
        {
            1,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
        VkAttachmentReference colorAttachment = 
        {
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        std::vector<SubpassParams> subpassParams = 
        {
            {
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                {},
                {colorAttachment},
                {},
                &depthAttachment,
                {},
            },
        };

        std::vector<VkSubpassDependency> subpassDependencies =
        {
            {
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_DEPENDENCY_BY_REGION_BIT
            },
            {
                0,
                VK_SUBPASS_EXTERNAL,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_DEPENDENCY_BY_REGION_BIT
            },
        };

        CreateRenderPass(device, attachmentDescriptions, subpassParams, subpassDependencies, renderPass);

        //graphics pipeline
        std::vector<VkPushConstantRange> range = 
        {
            {
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(float) * 8
            }
        };

        std::vector<VkDescriptorSetLayout> descriptorLayouts = { descriptorSetLayout };
        CreatePipelineLayout(device, descriptorLayouts, range, pipelineLayout);
         
        std::vector<unsigned char> vertexShader;
        if (!GetBinaryFileContents("shaders/BumpMapping/shaderSPIRV.vert.txt", vertexShader))
        {
            return false;
        }

        VkShaderModule vertexShaderModule;
        CreateShaderModule(device, vertexShader, vertexShaderModule);

        std::vector<unsigned char> fragmentShader;
        if (!GetBinaryFileContents("shaders/BumpMapping/shaderSPIRV.frag.txt", fragmentShader))
        {
            return false;
        }

        VkShaderModule fragmentShaderModule;
        CreateShaderModule(device, fragmentShader, fragmentShaderModule);

        std::vector<ShaderStageParams> shaderStageParams =
        {
            {
                VK_SHADER_STAGE_VERTEX_BIT,
                vertexShaderModule,
                "main",
                nullptr
            },
            {
                VK_SHADER_STAGE_FRAGMENT_BIT,
                fragmentShaderModule,
                "main",
                nullptr
            },
        };

        std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos;
        SpecifyPipelineShaderStages(shaderStageParams, shaderStageInfos);

        std::vector<VkVertexInputBindingDescription> vertexInputDescriptions = 
        {
            {
                0,
                14 * sizeof(float),
                VK_VERTEX_INPUT_RATE_VERTEX
            },
        };

        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescription =
        {
            //position
            {
                0, //location
                0, //binding
                VK_FORMAT_R32G32B32_SFLOAT, //format
                0, //offset
            },
            //normal
            {
                1,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                3 * sizeof(float),
            },
            //tex coords
            {
                2,
                0,
                VK_FORMAT_R32G32_SFLOAT,
                6 * sizeof(float),
            },
            //tangent
            {
                3,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                8 * sizeof(float),
            },
            //bitangent
            {
                4,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                11 * sizeof(float),
            },

        };

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
        SpecifyPipelineVertexInputState(vertexInputDescriptions, vertexAttributeDescription, vertexInputStateCreateInfo);

        VkPipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo;
        SpecifyPipelineInputAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false, assemblyStateCreateInfo);

        VkViewport viewport = {
            0.0f,
            0.0f,
            static_cast<float>(swapchain.size.width),
            static_cast<float>(swapchain.size.height),
            0.0f,
            1.0f,
        };

        VkOffset2D offset = { 0, 0 };
        VkRect2D rect = { offset, swapchain.size };

        ViewportInfo viewportInfo =
        {
            {viewport}, {rect}
        };

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
        SpecifyPipelineViewportAndScissorTestState(viewportInfo, viewportStateCreateInfo);

        VkPipelineRasterizationStateCreateInfo resterizationStateCreateInfo;
        SpecifyPipelineRasterizationState(false, false, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
            false, 0.0f, 0.0f, 0.0f, 1.0f, resterizationStateCreateInfo);

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
        SpecifyPipelineMultisamlpeState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, 0, false, false, multisampleStateCreateInfo);

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfoInfo;
        SpecifPipelineDepthAndStencilState(true, true, VK_COMPARE_OP_LESS_OR_EQUAL,
            false, 0.0f, 1.0f, false, {}, {}, depthStencilStateCreateInfoInfo);

        std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates =
        {
            {
                false,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_OP_ADD,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_OP_ADD,
                VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT
            }
        };

        VkPipelineColorBlendStateCreateInfo blendStateCreateInfo;
        SpecifyPipelineBlendState(false, VK_LOGIC_OP_COPY, attachmentBlendStates, {1.0f, 1.0f, 1.0f, 1.0f}, blendStateCreateInfo);

        std::vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
        SpecifyPipelineDynamicStates(dynamicStates, dynamicStateCreateInfo);

        VkGraphicsPipelineCreateInfo pipelineCreateInfo;
        SpecifyGraphicsPipelineParameters(0, shaderStageInfos, vertexInputStateCreateInfo, assemblyStateCreateInfo,
            nullptr, &viewportStateCreateInfo, resterizationStateCreateInfo, &multisampleStateCreateInfo, &depthStencilStateCreateInfoInfo,
            &blendStateCreateInfo, &dynamicStateCreateInfo, pipelineLayout, renderPass, 0, VK_NULL_HANDLE, -1, pipelineCreateInfo);

        std::vector<VkPipeline> pipelines;
        CreateGraphicsPipelines(device, {pipelineCreateInfo}, VK_NULL_HANDLE, pipelines);
        pipeline = pipelines[0];

        return true;
    }

    bool BumpMappingSample::Draw()
    {
        auto recordCommandBuffer = [&](VkCommandBuffer commandBuffer, uint32_t imageIndex, VkFramebuffer framebuffer)
        {
            BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);

            if (updateUniformBuffer)
            {
                BufferTransition preTransition =
                {
                    uniformBuffer,
                    VK_ACCESS_UNIFORM_READ_BIT,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                };
                SetBufferMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, { preTransition });
                VkBufferCopy region = {
                    0, 0, sizeof(uniformObject)
                };
                std::vector<VkBufferCopy> regions = { region };
                CopyDataBetweenBuffers(commandBuffer, stagingBuffer, uniformBuffer, regions);

                BufferTransition postTransition =
                {
                    uniformBuffer,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_ACCESS_UNIFORM_READ_BIT,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                };
                SetBufferMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, { postTransition });
            }

            if (presentQueue.familyIndex != graphicsQueue.familyIndex)
            {
                ImageTransition transitionBeforeDrawing =
                {
                    swapchain.images[imageIndex],
                    VK_ACCESS_MEMORY_READ_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    presentQueue.familyIndex,
                    graphicsQueue.familyIndex,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                };

                SetImageMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { transitionBeforeDrawing });
            }

            BeginRenderPass(commandBuffer, renderPass, framebuffer, { {0,0}, swapchain.size },
                { {0.1f, 0.2f, 0.3f, 1.0f}, {1.0f, 0} }, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = {
                0.0f,
                0.0f,
                static_cast<float>(swapchain.size.width),
                static_cast<float>(swapchain.size.height),
                0.0f,
                1.0f,
            };

            SetViewportStateDynamically(commandBuffer, 0, { viewport });

            VkOffset2D scissor = { 0, 0 };
            VkRect2D rect = { scissor, swapchain.size };
            SetScissorsStateDynamically(commandBuffer, 0, { rect });

            BindVertexBuffers(commandBuffer, 0, { {vertexBuffer, 0} });

            BindDescitorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSets, {});

            BindPipelineObject(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            std::array<float, 8> lightAndViewPosition = { 0.0, 0.0, 1.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.0f };
            ProvidePushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 8,
                &lightAndViewPosition[0]);

            for (size_t i = 0; i < model.meshes.size(); i++)
            {
                DrawGeometry(commandBuffer, model.meshes[i].vertexCount, 1, model.meshes[i].vertexOffset, 0);
            }

            EndRenderPass(commandBuffer);

            if (presentQueue.familyIndex != graphicsQueue.familyIndex)
            {
                ImageTransition transitionBeforeDrawing =
                {
                    swapchain.images[imageIndex],
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_ACCESS_MEMORY_READ_BIT,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    graphicsQueue.familyIndex,
                    presentQueue.familyIndex,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                };

                SetImageMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { transitionBeforeDrawing });
            }

            EndCommandBufferRecordingOperation(commandBuffer);
            return true;
        };

        std::vector<WaitSemaphoreInfo> waitInfos;
        IncreasePerformanceThroughtIncreasingTheNumberOfSeparatelyRenderedFrames(device, graphicsQueue.handle,
            presentQueue.handle, swapchain.handle, swapchain.size, swapchain.imageViews,
            renderPass, waitInfos, recordCommandBuffer, frameResources);

        return true;
    }

    bool BumpMappingSample::Resize()
    {
        return true;
    }

    void BumpMappingSample::Destroy()
    {
        WaitForAllSumbittedCommandsToBeFinished(device);
    }
}