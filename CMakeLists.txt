cmake_minimum_required(VERSION 3.22)
project(VulkanBook)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME VulkanBook)

include_directories(src)
include_directories(samples)

add_executable(VulkanBook
        #samples/VertexDiffuseLighting/main.cpp
        #samples/PixelDiffuseLighting/main.cpp
        samples/BumpMapping/main.cpp

        src/Library/Common/Log.cpp
        src/Library/Common/Log.h

        src/Library/Common/MeshLoader.cpp
        src/Library/Common/MeshLoader.h

        src/Library/Core/Core.h

        src/Library/Platform/Win32Window.cpp
        src/Library/Platform/Win32Window.h
        src/Library/Platform/WindowParams.h

        src/Library/Source/CommandBuffer.cpp
        src/Library/Source/CommandBuffer.h

        src/Library/Source/DebugMesenger.cpp
        src/Library/Source/DebugMesenger.h

        src/Library/Source/Instance.cpp
        src/Library/Source/Instance.h

        src/Library/Source/LogicalDevice.cpp
        src/Library/Source/LogicalDevice.h

        src/Library/Source/PhysicalDevice.cpp
        src/Library/Source/PhysicalDevice.h

        src/Library/Source/Swapchain.cpp
        src/Library/Source/Swapchain.h

        src/Library/Source/Resources.h
        src/Library/Source/Resources.cpp

        src/Library/Source/DescriptorSets.h
        src/Library/Source/DescriptorSets.cpp
        
        src/Library/Source/RenderPass.h
        src/Library/Source/RenderPass.cpp
        
        src/Library/Source/Pipeline.h
        src/Library/Source/Pipeline.cpp

        src/Library/Common/Tools.h 
        src/Library/Common/Tools.cpp

        src/Library/Source/Drawing.cpp
        src/Library/Source/Drawing.h
        
        samples/VulkanSampleBase.h
        samples/VulkanSampleBase.cpp
        
        samples/VertexDiffuseLighting/VertexDiffuseSample.h
        samples/VertexDiffuseLighting/VertexDiffuseSample.cpp

        samples/PixelDiffuseLighting/PixelDiffuseSample.h
        samples/PixelDiffuseLighting/PixelDiffuseSample.cpp

        samples/BumpMapping/BumpMappingSample.h
        samples/BumpMapping/BumpMappingSample.cpp

        src/Library/Structs/Buffer.h
        src/Library/Structs/Descriptors.h
        src/Library/Structs/Image.h
        src/Library/Structs/Pipeline.h
        src/Library/Structs/QueueInfo.h
        src/Library/Structs/Renderpass.h
        src/Library/Structs/Semaphore.h
        src/Library/Common/TextureLoader.h 
        src/Library/Common/TextureLoader.cpp

        external/stb_image.h 
        external/tiny_obj_loader.h
        external/stbi_image.cpp)

set(VULKAN_SDK_PATH  D:/VulkanSDK/1.3.216.0)
set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")

message(STATUS "Vulkan path is ${VULKAN_SDK_PATH}")
message(STATUS "Vulkan include is ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan libs is ${Vulkan_LIBRARIES}")

target_include_directories(${PROJECT_NAME} PUBLIC
        ${Vulkan_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME} PUBLIC
        ${Vulkan_LIBRARIES})

target_link_libraries(${PROJECT_NAME} vulkan-1)

add_subdirectory(external/glm)
target_link_libraries(${PROJECT_NAME} glm)

add_subdirectory(submodules/spdlog)
target_link_libraries(${PROJECT_NAME} spdlog)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/shaders/ ${PROJECT_BINARY_DIR}/shaders/)