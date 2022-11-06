#include "VulkanEntry.h"

namespace vk
{
    VulkanEntry::VulkanEntry()
    {
#ifdef NDEBUG
    debugModeEnabled = false;
#else
    debugModeEnabled = true;
#endif
    }

    void VulkanEntry::Destroy()
    {
        swapchain.Destroy(device.GetDevice(), instance.GetInstance());
        device.Destroy();
        debugMessenger.Destroy(instance.GetInstance());
        instance.Destroy();
    }

    void VulkanEntry::Run(int width, int height)
    {
        this->width = width;
        this->height = height;

        //querry layers for instance and logical device
        QuerryAvailableLayers();

        //creating instance with layers and extensions
        instance.CreateInstance(debugMessenger.GetDebugUtilsMessengerInfo(), debugModeEnabled,
                                usedValidationLayers, availableLayers, usedInstanceExtensions);

        //creating debug messenger to log validation layers info
        debugMessenger.CreateDebugMesenger(instance.GetInstance(), debugModeEnabled);

        //pick up suitable physical device
        gpu.PickUpPhysicalDevice(instance.GetInstance());

        //creating logical device
        device.createLogicalDevice(gpu.GetGPU(), usedDeviceExtensions, usedValidationLayers, 
                                gpu.IsQueuesIdentic(), gpu.GetQueuesInfo(), debugModeEnabled, 
                                gpu.GetGraphicsQueue(), gpu.GetComputeQueue());
        
        //window system creation
        window.CreateWindowsWindow(width, height);
        swapchain.Init(instance.GetInstance(), gpu.GetGPU(), gpu.GetQueuesInfo(), window.GetWindowParams());
        swapchain.CreateSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, 
                                {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, 
                                width, height, device.GetDevice(), gpu.GetGPU());
    }

    void VulkanEntry::QuerryAvailableLayers()
    {
        if(!debugModeEnabled)
        {
            LOG_INFO("Debug mode disabled, no layers will be enabled");
            return;
        }

        vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);
        if(availableLayersCount == 0)
        {
            LOG_INFO("Cannot querry layers");
        }

        availableLayers.resize(availableLayersCount);
        vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());
    }

    void VulkanEntry::Update()
    {
        swapchain.PresentImage(device.GetDevice(), gpu.GetGraphicsQueue());
    }

	bool VulkanEntry::IsRunning()
    {
        return true;
    }
}

