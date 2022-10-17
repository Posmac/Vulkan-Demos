cmake_minimum_required(VERSION 3.22)
project(VulkanBook)

set(CMAKE_CXX_STANDARD 17)
set(GLFW_VULKAN_STATIC, "True")
set(PROJECT_NAME VulkanBook)

include_directories(src)
include_directories(src/Core)
include_directories(src/vk)
include_directories(src/Platform)
include_directories(src/Common)

add_executable(VulkanBook
        src/App.cpp
        src/Core/Core.h
        src/Common/QueueInfo.h
        src/Platform/Win32Window.cpp
        src/Platform/Win32Window.h
        src/vk/vkApp.cpp
        src/vk/vkApp.h
        src/vk/vkDebugMesenger.cpp
        src/vk/vkDebugMesenger.h
        src/vk/vkInstance.cpp
        src/vk/vkInstance.h
        src/vk/vkPhysicalDevice.cpp
        src/vk/vkPhysicalDevice.h
        src/vk/vkLogicalDevice.cpp
        src/vk/vkLogicalDevice.h
        src/vk/vkSwapchain.cpp
        src/vk/vkSwapchain.h

        external/stb_image.h 
        external/tiny_obj_loader.h)

set(VULKAN_SDK_PATH  D:/VulkanSDK/1.3.216.0)
set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")

include_directories(shaders)

message(STATUS "Vulkan path is ${VULKAN_SDK_PATH}")
message(STATUS "Vulkan include is ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan libs is ${Vulkan_LIBRARIES}")

target_include_directories(${PROJECT_NAME} PUBLIC
        ${Vulkan_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME} PUBLIC
        ${Vulkan_LIBRARIES})

target_link_libraries(${PROJECT_NAME} vulkan-1)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)