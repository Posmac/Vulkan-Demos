#include "VulkanSample.h"

namespace vk
{
    bool Sample::InitVulkan(WindowParameters& windowParams,
        std::vector<const char*> validationLayer,
        std::vector<const char*> instanceExtensions,
        std::vector<const char*> deviceExtensions,
        VkPhysicalDeviceFeatures* deviceFeatures,
        bool useDepth,
        VkImageUsageFlags swapChainImageUsage,
        VkImageUsageFlags depthImageUsage)
    {
        //creating instance with layers and extensions
        CreateVulkanInstance(instanceExtensions, validationLayer, true, "Vulkan Framework", instance);

        //create debugger
        CreateDebugMesenger(instance, true, messenger);

        //creating surface
        CreatePresentationSurface(instance, windowParams, presentationSurface);

        //picking gpu
        std::vector<VkPhysicalDevice> physicalDevices;
        EnumerateAvailablePhysicalDevices(instance, physicalDevices);

        for (auto& gpu : physicalDevices)
        {
            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(gpu, VK_QUEUE_GRAPHICS_BIT, graphicsQueue.familyIndex))
            {
                continue;
            }

            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(gpu, VK_QUEUE_COMPUTE_BIT, computeQueue.familyIndex))
            {
                continue;
            }

            if (!SelectQueueFamilyThatSupportsPresentationToGivenSurface(gpu, presentationSurface, presentQueue.familyIndex))
            {
                continue;
            }

            std::vector<QueueInfo> requstedQueues = { {graphicsQueue.familyIndex, {1.0f}} };
            if (graphicsQueue.familyIndex != computeQueue.familyIndex)
            {
                requstedQueues.push_back({ computeQueue.familyIndex, {1.0f} });
            }
            if (graphicsQueue.familyIndex != presentQueue.familyIndex &&
                computeQueue.familyIndex != presentQueue.familyIndex)
            {
                requstedQueues.push_back({ presentQueue.familyIndex, {1.0f} });
            }

            if (!CreateLogicalDevice(gpu, deviceExtensions, validationLayer, requstedQueues,
                deviceFeatures, true, device))
            {
                continue;
            }

            physicalDevice = gpu;
            GetDeviceQueue(device, graphicsQueue.familyIndex, 0, graphicsQueue.handle);
            GetDeviceQueue(device, computeQueue.familyIndex, 0, graphicsQueue.handle);
            GetDeviceQueue(device, presentQueue.familyIndex, 0, graphicsQueue.handle);
            break;
        }

        if (device == VK_NULL_HANDLE)
        {
            WARN_LOG("No device was created");
            return false;
        }

        //creating command pool
        CreateCommandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue.familyIndex, commandPool);

        //preparing frameresources
        for (uint32_t i = 0; i < framesCount; i++)
        {
            std::vector<VkCommandBuffer> commandBuffer;
            VkSemaphore imageAcquiredSemaphore;
            VkSemaphore readyToPresentSemaphore;
            VkFence drawingFinishedFence;
            VkImageView depthAttachment;
            VkFramebuffer framebuffer;

            AllocateCommandBuffers(device, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                1, commandBuffer);
            CreateVkSemaphore(device, imageAcquiredSemaphore);
            CreateVkSemaphore(device, readyToPresentSemaphore);
            CreateVkFence(device, true, drawingFinishedFence);

            frameResources.push_back(
                {
                    commandBuffer[0],
                    std::move(imageAcquiredSemaphore),
                    std::move(readyToPresentSemaphore),
                    std::move(drawingFinishedFence),
                    std::move(depthAttachment),
                    std::move(framebuffer)
                }
            );
        }

        swapchain.format = VK_FORMAT_R8G8B8A8_UNORM;
        swapchain.size = { 800, 600 };
        swapchain.swapchain = VK_NULL_HANDLE;
        swapchain.images.clear();
        swapchain.imageViews.clear();
        depthImages.clear();
        depthImageViews.clear();
        depthImageMemory.clear();

        if (!CreateSwapchain(useDepth, swapChainImageUsage, depthImageUsage))
        {
            return false;
        }

        return true;
    }

    bool Sample::CreateSwapchain(bool useDepth,
        VkImageUsageFlags swapChainImageUsage,
        VkImageUsageFlags depthImageUsage)
    {
        WaitForAllSumbittedCommandsToBeFinished(device);

        isReady = false;

        VkSwapchainKHR oldswapchain = swapchain.swapchain;

        if (!CreateSwapchainWithRGBA8FormatAndMailBoxPresentMode(device, physicalDevice, presentationSurface,
            swapChainImageUsage, swapchain.size, swapchain.format, oldswapchain, swapchain.swapchain, swapchain.images))
        {
            return false;
        }

        swapchain.imageViews.resize(swapchain.images.size());
        for (int i = 0; i < swapchain.images.size(); i++)
        {
            CreateImageView(device, swapchain.images[i], VK_IMAGE_VIEW_TYPE_2D, swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT,
                swapchain.imageViews[i]);
        }

        depthImages.resize(framesCount);
        depthImageViews.resize(framesCount);
        depthImageMemory.resize(framesCount);

        for (int i = 0; i < framesCount; i++)
        {
            Create2DImageAndView(device, physicalDevice, depthFormat, swapchain.size, 1, 1,
                VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
                depthImages[i], depthImageMemory[i], depthImageViews[i]);
            frameResources[i].depthAttachment = depthImageViews[i];
        }

        isReady = true;
        return true;
    }

    bool Sample::Initialize(WindowParameters& windowParams,
        std::vector<const char*> validationLayer,
        std::vector<const char*> instanceExtensions,
        std::vector<const char*> deviceExtensions)
    {
        if (!InitVulkan(windowParams, validationLayer, instanceExtensions, deviceExtensions, nullptr,
            false, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
        {
            return false;
        }

        //load mesh data to memory
        model = MeshLoader::LoadMesh("models/teapot.obj", true, false);
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
        VkDeviceSize uniformBufferSize = 32 * sizeof(float);
        CreateBuffer(device, uniformBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
        AllocateAndBindMemoryObjectToBuffer(device, physicalDevice, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            stagingBufferMemory);

        CreateUniformBuffer(device, physicalDevice, uniformBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffer, UniformBufferMemory);

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0, 0, -1), glm::vec3(0), glm::vec3(0, 1, 0));
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f), 
            static_cast<float>(swapchain.size.width) / static_cast<float>(swapchain.size.height),
            0.01f, 100.0f);
        glm::mat4 modelView = viewMatrix * modelMatrix;

        std::vector<glm::mat4> matrices = { modelView, projectionMatrix };

        MapUpdateAndUnmapHostVisibleMemory(device, stagingBufferMemory, 0, uniformBufferSize,
            matrices.data(), true, nullptr);

        //descriptor set with uniform buffer
        VkDescriptorSetLayoutBinding descriptorSetLayoutBind =
        {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            nullptr
        };
        CreateDescriptorSetLayout(device, { descriptorSetLayoutBind }, descriptorSetLayout);

        VkDescriptorPoolSize descriptorPoolSize = {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1
        };
        CreateDescriptorPool(device, false, 1, { descriptorPoolSize }, descriptorPool);
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

        UpdateDescriptorSets(device, {}, { bufferDescriptorUpdate }, {}, {});

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
                depthAttachment,
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
        std::vector<VkPushConstantRange> range;
        std::vector<VkDescriptorSetLayout> descriptorLayouts = { descriptorSetLayout };
        CreatePipelineLayout(device, descriptorLayouts, range, pipelineLayout);

        std::vector<unsigned char> vertexShader;
        if (!GetBinaryFileContents("shaders/shader11_1_vert", vertexShader))
        {
            return false;
        }
        VkShaderModule vertexShaderModule;
        CreateShaderModule(device, vertexShader, vertexShaderModule);

        std::vector<unsigned char> fragmentShader;
        if (!GetBinaryFileContents("shaders/shader11_1_frag", fragmentShader))
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
                6 * sizeof(float),
                VK_VERTEX_INPUT_RATE_VERTEX
            },
        };

        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescription =
        {
            {
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                0,
            },
            {
                1,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                3 * sizeof(float),
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

    bool Sample::Draw()
    {
        auto recordCommandBuffer = [&](VkCommandBuffer commandBuffer, uint32_t imageIndex, VkFramebuffer framebuffer)
        {
            BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);

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
            presentQueue.handle, swapchain.swapchain, swapchain.size, swapchain.imageViews,
            renderPass, waitInfos, recordCommandBuffer, frameResources);

        return true;
    }

    bool Sample::Resize()
    {
        return true;
    }

    void Sample::Destroy()
    {
        WaitForAllSumbittedCommandsToBeFinished(device);
    }
}