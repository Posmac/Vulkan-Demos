#pragma once

#include "vkDebugMesenger.h"
#include "vkInstance.h"
#include "vkLogicalDevice.h"
#include "vkPhysicalDevice.h"
#include "vkSwapchain.h"
#include "Win32Window.h"

namespace vk
{
    class VkApp
    {
    public:
        VkApp();
        void Destroy();
		void Run(int width, int height);
		bool IsRunning();
		void Update();
	private:
        void QuerryAvailableLayers();
    private:
		int width;
		int height;

		uint32_t availableLayersCount;
		std::vector<VkLayerProperties> availableLayers;

        bool debugModeEnabled;
        CVkDebugMessenger debugMessenger;
		CVkInstance instance; 
		CVkPhysicalDevice gpu;
		CVkLogicalDevice device;
		CVkSwapchain swapchain;
		Win32Window window;

    private:
        //TODO: abstract in some structure later
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
    };
}