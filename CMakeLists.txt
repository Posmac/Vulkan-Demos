cmake_minimum_required(VERSION 3.22)
project(VulkanBook)

set(CMAKE_CXX_STANDARD 17)
set(GLFW_VULKAN_STATIC, "True")
set(PROJECT_NAME VulkanBook)

add_executable(VulkanBook
        src/main.cpp
        external/stb_image.h 
        external/tiny_obj_loader.h 
        src/Vulkan.h
        src/Vulkan.cpp
        src/Core/Core.h)

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