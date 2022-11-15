#include "VertexDiffuse.h"

namespace vk
{
    VertexDiffuseExample::VertexDiffuseExample()
    {
    }
    VertexDiffuseExample::VertexDiffuseExample(const std::vector<const char*>& layers,
        const std::vector<const char*>& instanceExtensions,
        const std::vector<const char*>& deviceExtensions,
        bool isDebugModeEnabled,
        int width,
        int height,
        WindowParameters& windowsParams)
    {
        //1.0 Get available instance layer properties
        std::vector<VkLayerProperties> availableLayerProperties;
        uint32_t availableLayerPropertiesCount;
        EnumerateAvailableLayerProperties(availableLayerPropertiesCount, availableLayerProperties,
            isDebugModeEnabled);

        //1.1 based on instance layers and extensions - create instance
        instance.CreateInstance(debugMessenger.GetDebugUtilsMessengerInfo(), isDebugModeEnabled,
            layers, availableLayerProperties, instanceExtensions);

        //1.2 create debug messenger info
        debugMessenger.CreateDebugMesenger(instance.GetInstance(), isDebugModeEnabled);

        //1.3 pickup physical device
        VkPhysicalDevice vkGpu = gpu.PickUpPhysicalDevice(instance.GetInstance());

        //1.4 based on gpu , extensions and layers - create logical device
        VkDevice logicalDevice = device.CreateLogicalDevice(vkGpu, deviceExtensions, layers, gpu.IsQueuesIdentic(),
            gpu.GetQueuesInfo(), isDebugModeEnabled);

        gpu.AcquireQueues(logicalDevice);

        //2 Get graphics and compute queues handle
        queueInfo = gpu.GetQueuesInfo();

        //3 create a swapchain
        swapchain.Init(instance.GetInstance(), vkGpu, queueInfo, windowsParams);
        VkSurfaceFormatKHR format{};
        format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        format.format = VK_FORMAT_R8G8B8A8_SNORM;
        swapchain.CreateSwapchain(logicalDevice, vkGpu, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, format, width, height);

        //4 acquire swapchaing images 
        std::vector<VkImage> images = swapchain.GetImages();

        //5 create swapchaing images view
        for (int i = 0; i < images.size(); i++)
        {
            VkImageView view = entry.CreateImageView(logicalDevice, images[i], VK_IMAGE_VIEW_TYPE_2D, format.format,
                VK_IMAGE_ASPECT_COLOR_BIT);
            swapChainImageViews.push_back(view);
        }

        //6 create set of resources required to generate frames
        VkCommandPool vkPool = commandPool.CreateCommandPool(logicalDevice, gpu.GetQueuesInfo().graphicsFamily.value());
        std::vector<VkCommandBuffer> vkCommandBuffers = commandBuffer.CreateCommandBuffers(logicalDevice, vkPool, 1);

        VkExtent3D imageSize{};
        imageSize.depth = 1;
        imageSize.height = height;
        imageSize.width = width;

        //Create frame resource

        VkSemaphore imageAcquiredSemaphore;
        VkSemaphore readyToPresentSemaphore;
        VkFence drawingFinishedFence;
        VkFramebuffer frameBuffer;
        commandBuffer.CreateVkSemaphore(device.GetDevice(), imageAcquiredSemaphore);
        commandBuffer.CreateVkSemaphore(device.GetDevice(), readyToPresentSemaphore);
        commandBuffer.CreateFence(device.GetDevice(), VK_FENCE_CREATE_SIGNALED_BIT, drawingFinishedFence);

        framesResources = {
            vkCommandBuffers[0],
            std::move(imageAcquiredSemaphore),
            std::move(readyToPresentSemaphore),
            std::move(drawingFinishedFence),
            {}
        };

        //7 Load 3D model with data into Mesh structure
        std::string path = "models/knot.obj";
        model = MeshLoader::LoadMesh(path, true, true);

        //8 Create buffer for mesh data
        VkDeviceSize size = sizeof(float) * model.data.size();
        vertexBuffer = entry.CreateBuffer(logicalDevice, size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_SHARING_MODE_EXCLUSIVE);

        //9 Allocate memory for the mesh buffer
        vertexBufferMemory = entry.AllocateBufferMemory(vkGpu, logicalDevice,
            vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        //10 Copy vertex data from mesh.data using stage buffer
        std::vector<VkSemaphore>semaphores;
        entry.UpdateBufferUsingStagingBuffer(logicalDevice,
            vkGpu, model.data.data(), size, vkCommandBuffers[0], vertexBuffer,
            0, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            queueInfo.graphicsQueue, semaphores);

        //11 Create uniform buffer to hold matrices
        VkBuffer matrixBuffer = entry.CreateUniformBuffer(logicalDevice, vkGpu,
            sizeof(float) * 32, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        //12 Create description set layout
        std::vector< VkDescriptorSetLayoutBinding> binds =
        {
            {
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                VK_SHADER_STAGE_VERTEX_BIT,
                nullptr
            }
        };

        //13 Create descriptor pool
        std::vector<VkDescriptorPoolSize> descriptorTypes =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
        };

        VkDescriptorPool descriptorPool = entry.CreateDescriptorPool(logicalDevice, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            1, descriptorTypes);

        //14 Allocate descriptor sets
        VkDescriptorSetLayout layouts = entry.CreateDescriptorSetLayout(logicalDevice, binds);
        std::vector<VkDescriptorSet> descriptorSets = entry.AllocateDescriptorSet(logicalDevice, descriptorPool, { layouts });

        //15 Update descriptor set with uniform buffer

        VkDescriptorBufferInfo updateInfo{};
        updateInfo.buffer = matrixBuffer;
        updateInfo.offset = 0;
        updateInfo.range = VK_WHOLE_SIZE;

        BufferDescriptorInfo uniformBufferInfo{};
        uniformBufferInfo.arrayElement = 0;
        uniformBufferInfo.binding = 0;
        uniformBufferInfo.descriptorSet = descriptorSets[0];
        uniformBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformBufferInfo.bufferInfo = { updateInfo };

        std::vector<ImageDescriptorInfo> imageDescriptorsInfo;
        std::vector<BufferDescriptorInfo> bufferDescriptorsInfo = { uniformBufferInfo };
        std::vector<TexelBufferDescriptorInfo> texelBufferDescriptorsInfo;
        std::vector<CopyDescriptorInfo> copyDescriptorsInfo;

        entry.UpdateDecsriptorSets(logicalDevice, imageDescriptorsInfo, bufferDescriptorsInfo,
            texelBufferDescriptorsInfo, copyDescriptorsInfo);

        //16 Prepare params for render pass creaion
        std::vector<VkAttachmentDescription> attachments =
        {
            entry.SpecifyColorAttachmentDescription(),
            entry.SpecifyDepthAttachmentDescription()
        };

        //17 Specify one subpass with 2 attachments
        std::vector<SubpassParams> params(1);
        params[0].pipelineType = VK_PIPELINE_BIND_POINT_GRAPHICS;
        params[0].inputAttachments = {};
        params[0].colorAttachments =
        {
            {
                0,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            }
        };
        params[0].depthStencilAttachment =
        {
            1,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
        params[0].preserveAttachments = {};
        params[0].resolveAttachments = {};

        std::vector<VkSubpassDescription> subpassDescriptions = entry.SpecifySubpassDescriptions(params);

        //18-19 Specify 2 supbass dependencies
        std::vector<VkSubpassDependency> subpassDeps =
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
            }
        };

        //20 Create a render pass
        entry.CreateRenderPass(logicalDevice, attachments, subpassDescriptions, subpassDeps, renderPass);

        //21 Create pipeline layout
        entry.CreatePipelineLayout(logicalDevice, { layouts }, {}, pipelineLayout);

        //22-23.Create vertex shader module and fragment shader module
        std::vector<unsigned char> shaderSourceCode;
        entry.GetBinaryFileContents("shaders/shader11_1_vert", shaderSourceCode);
        VkShaderModule vertexModule = entry.CreateShaderModule(logicalDevice, shaderSourceCode);
        entry.GetBinaryFileContents("shaders/shader11_1_frag", shaderSourceCode);
        VkShaderModule fragModule = entry.CreateShaderModule(logicalDevice, shaderSourceCode);

        //24 Specify shader stages in pipeline
        std::vector<ShaderStageParams> shaderParams(2);
        shaderParams[0].entryPoint = "main";
        shaderParams[0].shaderModule = vertexModule;
        shaderParams[0].shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderParams[0].specialisationInfo = nullptr;

        shaderParams[1].entryPoint = "main";
        shaderParams[1].shaderModule = fragModule;
        shaderParams[1].shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderParams[1].specialisationInfo = nullptr;

        std::vector<VkPipelineShaderStageCreateInfo> shaderInfos{};
        for (auto& param : shaderParams)
        {
            VkPipelineShaderStageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = 0;
            info.module = param.shaderModule;
            info.pName = param.entryPoint;
            info.pSpecializationInfo = param.specialisationInfo;
            info.stage = param.shaderStage;
            shaderInfos.push_back(info);
        }

        //25-26. Specify pipeline vertex input state
        std::vector<VkVertexInputAttributeDescription> attribDescriptions = {
            {
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                0
            },
            {
                1,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                3 * sizeof(float)
            }
        };

        std::vector<VkVertexInputBindingDescription> bindDescriptions = {
            {
                0,
                6 * sizeof(float),
                VK_VERTEX_INPUT_RATE_VERTEX
            }
        };

        VkPipelineVertexInputStateCreateInfo inputState{};
        entry.SpecifyBindingAttributeDescriptions(bindDescriptions, attribDescriptions, inputState);

        //27 Specify input assembly state
        VkPipelineInputAssemblyStateCreateInfo assemblyState = entry.SpecifyPipelineInputAssemblyState(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        //28. Specify viewport and scissor
        VkOffset2D offset = { 0, 0 };
        VkExtent2D extent = { width, height };
        scirssor = { offset , extent };
        VkViewport viewPort = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };
        ViewportInfo info = { {viewPort}, {scirssor} };
        VkPipelineViewportStateCreateInfo viewportState{};
        entry.SpecifyViewportAndScissorsState(info, viewportState);

        //29. Specify rasterization state
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        entry.SpecifyRasterizationState(false, false,
            VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, false, 0.0f, 0.0f,
            0.0f, 1.0f, rasterizationState);

        //30. Specify pipeline multisample
        VkPipelineMultisampleStateCreateInfo msState{};
        entry.SpecifyMultisamplingState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f,
            nullptr, false, false, msState);

        //31. Specify pipeline depth state
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        entry.SpecifyDepthStencilState(true, true, VK_COMPARE_OP_LESS_OR_EQUAL, false, false, {}, {}, 0, 0, depthStencilState);

        //32. Specify pipeline blend state
        std::vector<VkPipelineColorBlendAttachmentState> attachment_blend_states = {
            {
                false,                          // VkBool32                 blendEnable
                VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcColorBlendFactor
                VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstColorBlendFactor
                VK_BLEND_OP_ADD,                // VkBlendOp                colorBlendOp
                VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcAlphaBlendFactor
                VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstAlphaBlendFactor
                VK_BLEND_OP_ADD,                // VkBlendOp                alphaBlendOp
                VK_COLOR_COMPONENT_R_BIT |      // VkColorComponentFlags    colorWriteMask
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT
            }
        };

        float blend[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
        VkPipelineColorBlendStateCreateInfo blendInfo{};
        entry.SpecifyColorBlendState(attachment_blend_states, false, VK_LOGIC_OP_COPY, blend, blendInfo);

        //33. Specify dynamic states
        std::vector<VkDynamicState> dynStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicStates{};
        dynamicStates.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStates.pNext = nullptr;
        dynamicStates.flags = 0;
        dynamicStates.dynamicStateCount = static_cast<uint32_t>(dynStates.size());
        dynamicStates.pDynamicStates = dynStates.data();
        // VkPipelineDynamicStateCreateInfo dynamicStates = entry.SpecifyDynamicStates(dynStates);

         //34. Create graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                nullptr,
                {},
                static_cast<uint32_t>(shaderInfos.size()),
                shaderInfos.data(),
                &inputState,
                &assemblyState,
                {},
                &viewportState,
                &rasterizationState,
                &msState,
                &depthStencilState,
                &blendInfo,
                &dynamicStates,
                pipelineLayout,
                renderPass,
                0,
                {},
                0
        };

        entry.CreateGraphicsPipeline(logicalDevice, pipelineInfo, {}, pipeline);

        //35. Create stage buffer for matrices
        uniformBuffer = entry.CreateBuffer(logicalDevice, sizeof(float) * 16, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_SHARING_MODE_EXCLUSIVE);

        uniformBufferMemory = entry.AllocateBufferMemory(vkGpu, logicalDevice, uniformBuffer,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        //36. Create depth image

        depthImage = entry.CreateImage(logicalDevice, VK_IMAGE_TYPE_2D, VK_FORMAT_D16_UNORM, imageSize, 1, 1, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        depthImagesMemory = entry.AllocateImageMemory(vkGpu, logicalDevice, depthImage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        depthImageView = entry.CreateImageView(logicalDevice, depthImage,
            VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT);

        //37-38. Create model matrix and perspective one
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, 0, 10));

        glm::mat4 perspective = glm::perspective(glm::radians(60.0f), (float)width / height, 0.1f, 100.0f);
        glm::mat4 projection = glm::lookAt(glm::vec3(1.0f), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 perproj = projection * perspective;

        entry.MapUpdateAndUnmapHostVisibleMemory(logicalDevice, uniformBufferMemory, 0, sizeof(float) * 16, &perproj[0]);
    }

    void VertexDiffuseExample::AcquireSwapchainImage(VkDevice device, VkSwapchainKHR swapchain,
        VkSemaphore semaphore, VkFence fence, uint32_t& index)
    {
        VK_CHECK_RESULT(vkAcquireNextImageKHR(device, swapchain,
            2000000000, semaphore, fence, &index));
    }

    void VertexDiffuseExample::PrepareAnimationFrame(VkDevice device,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkSwapchainKHR swapchain,
        VkExtent2D swapchainSize,
        std::vector<VkImageView>& swapChainViews,
        VkImageView depthAttachment,
        std::vector<WaitSemaphoreInfo> wainInfo,
        FrameResources& frameResource,
        VkRenderPass renderPass)
    {
        uint32_t imageIndex;
        AcquireSwapchainImage(device, swapchain, frameResource.imageAcquiredSemaphore, VK_NULL_HANDLE,
            imageIndex);

        std::vector<VkImageView> attachements = { swapChainViews[imageIndex],
                    depthAttachment };

        frameResource.framebuffer = entry.CreateFramebuffer(device,
            renderPass, attachements, swapchainSize, 1);

        RecordCommandBuffer(frameResource.commandBuffer, imageIndex, frameResource.framebuffer);

        std::vector<WaitSemaphoreInfo> waitInfoSemaphore = wainInfo;
        waitInfoSemaphore.push_back({
            frameResource.imageAcquiredSemaphore,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            });

        entry.SubmitCommandBuffersToQueue(graphicsQueue, waitInfoSemaphore,
            { frameResource.commandBuffer }, { frameResource.readyToPresentSemaphore },
            frameResource.drawingFinishedFence);

        PresentInfo present_info = {
            swapchain,
            imageIndex
        };

        PresentImage(presentQueue, { frameResource.readyToPresentSemaphore }, { present_info });
    }

    void VertexDiffuseExample::RecordCommandBuffer(VkCommandBuffer commandBuffer,
        uint32_t imageIndex,
        VkFramebuffer framebuffer)
    {
        entry.BeginCommandBufferRecord(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        if (!gpu.IsQueuesIdentic())
        {
            ImageTransition transitionBeforeDrawing =
            {
                (swapchain.GetImages())[imageIndex],
                VK_ACCESS_MEMORY_READ_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                queueInfo.computeFamily.value(),
                queueInfo.graphicsFamily.value(),
                VK_IMAGE_ASPECT_COLOR_BIT
            };

            entry.SetupImageMemoryBarrier(transitionBeforeDrawing);
        }

        entry.BeginRenderPass(renderPass, commandBuffer, framebuffer,
            scirssor, { {0.1f, 0.2f, 0.3f, 1.0f}, {1.0f, 0.0f} },
            VK_SUBPASS_CONTENTS_INLINE);

        entry.SetupScissorsStateDynamically(commandBuffer,
            0, { scirssor });

        entry.BindVertexBuffers(commandBuffer, { {vertexBuffer, 0} }, 0);
        entry.BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, descriptorSets, 0);
        entry.BindPipeline(commandBuffer, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);

        for (size_t i = 0; i < model.meshes.size(); i++)
        {
            entry.Draw(commandBuffer, model.meshes[i].vertexCount, 1, model.meshes[i].vertexOffset, 0.);
        }

        entry.EndRenderPass(commandBuffer);

        if (!gpu.IsQueuesIdentic())
        {
            ImageTransition transitionBeforePresent =
            {
                (swapchain.GetImages())[imageIndex],
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                queueInfo.computeFamily.value(),
                queueInfo.graphicsFamily.value(),
                VK_IMAGE_ASPECT_COLOR_BIT
            };

            entry.SetupImageMemoryBarrier(transitionBeforePresent);
        }

        entry.EndCommandBufferRecord(commandBuffer);
    }

    void VertexDiffuseExample::Draw()
    {
        //39-40 Draw
        PrepareAnimationFrame(device.GetDevice(), queueInfo.graphicsQueue, queueInfo.graphicsQueue,
            swapchain.GetSwapchain(), swapchain.GetExtent(), swapChainImageViews,
            depthImageView, {}, framesResources, renderPass);
    }

    void VertexDiffuseExample::PresentImage(VkQueue queue, std::vector<VkSemaphore> renderingSemaphores,
        std::vector<PresentInfo> imagesToPresent)
    {
        std::vector<VkSwapchainKHR> swapchains;
        std::vector<uint32_t> image_indices;

        for (auto& image_to_present : imagesToPresent) {
            swapchains.emplace_back(image_to_present.Swapchain);
            image_indices.emplace_back(image_to_present.ImageIndex);
        }

        VkPresentInfoKHR presentInfo = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,nullptr,
            static_cast<uint32_t>(renderingSemaphores.size()),
            renderingSemaphores.data(),
            static_cast<uint32_t>(swapchains.size()),
            swapchains.data(),
            image_indices.data(),
            nullptr
        };

        VK_CHECK_RESULT(vkQueuePresentKHR(queue, &presentInfo));
    }

    void VertexDiffuseExample::EnumerateAvailableLayerProperties(uint32_t& availableLayersCount,
        std::vector<VkLayerProperties>& availableLayerProperties, bool isDebugModeEnabled)
    {
        if (!isDebugModeEnabled)
        {
            WARN_LOG("Debug mode disabled, no layers will be enabled");
            return;
        }

        VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr));
        if (availableLayersCount == 0)
        {
            ERROR_LOG("Cannot querry layers");
        }

        availableLayerProperties.resize(availableLayersCount);

        VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&availableLayersCount,
            availableLayerProperties.data()));
    }
}

