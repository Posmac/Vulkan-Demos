#include "VertexDiffuse.h"
#include "Win32Window.h"
#include "Core/Core.h"

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
    vk::WindowParameters params = window.GetWindowParams();

	vk::VertexDiffuseExample example(usedValidationLayers, usedInstanceExtensions, usedDeviceExtensions,
        true, 800, 600, params);

    window.Render(example);
}
