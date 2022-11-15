cmake_minimum_required(VERSION 3.22)
project(VulkanBook)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME VulkanBook)

include_directories(src)
include_directories(src/Core)
include_directories(src/vk)
include_directories(src/Platform)
include_directories(src/Common)
include_directories(src/Examples)
include_directories(src/Structs)
include_directories(shaders)

add_executable(VulkanBook
        src/Application.cpp
        src/Core/Core.h
        src/Common/QueueInfo.h
        src/Platform/Win32Window.cpp
        src/Platform/Win32Window.h
        src/Platform/WindowParams.h
        src/vk/VulkanEntry.cpp
        src/vk/VulkanEntry.h
        src/vk/DebugMesenger.cpp
        src/vk/DebugMesenger.h
        src/vk/Instance.cpp
        src/vk/Instance.h
        src/vk/PhysicalDevice.cpp
        src/vk/PhysicalDevice.h
        src/vk/LogicalDevice.cpp
        src/vk/LogicalDevice.h
        src/vk/Swapchain.cpp
        src/vk/Swapchain.h
        src/vk/CommandPool.cpp
        src/vk/CommandPool.h
        src/vk/CommandBuffer.cpp
        src/vk/CommandBuffer.h

        src/Core/Log.h 
        src/Core/Log.cpp
        src/Structs/Image.h 
        src/Structs/Buffer.h
        src/Structs/Semaphore.h
        src/Structs/Descriptors.h 
        src/Structs/RenderPass.h 
        src/Structs/Pipeline.h 
        src/Common/MeshLoader.h
        src/Common/MeshLoader.cpp

        src/Examples/VertexDiffuse.h
        src/Examples/VertexDiffuse.cpp

        external/stb_image.h 
        external/tiny_obj_loader.h)

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