#include <vector>

#include "Library/Common/Log.h"
#include "PixelDiffuseSample.h"
#include "Library/Platform/Win32Window.h"
#include "Library/Platform/WindowParams.h"

int main()
{
    vk::Log::Init();
    ERROR_LOG("Logger initialized");

    const std::vector<const char*> usedValidationLayers = {
    "VK_LAYER_KHRONOS_validation",
    };

    const std::vector<const char*> usedInstanceExtensions =
    {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils"
    };

    const std::vector<const char*> usedDeviceExtensions = {
        "VK_KHR_swapchain",
    };

    vk::Win32Window window;
    window.CreateWindowsWindow(800, 600);

    vk::PixelDiffuseSample sample;
    sample.Initialize(window.GetWindowParams(), usedValidationLayers, usedInstanceExtensions, usedDeviceExtensions);

    window.Render(sample);
}
