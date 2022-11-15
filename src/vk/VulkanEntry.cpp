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
        device.CreateLogicalDevice(gpu.GetGPU(), usedDeviceExtensions, usedValidationLayers, 
                                gpu.IsQueuesIdentic(), gpu.GetQueuesInfo(), debugModeEnabled);

        
    }

    //moved to VertexDiffuse;
    void VulkanEntry::QuerryAvailableLayers() {  }

    void VulkanEntry::Update()
    {
    }

	bool VulkanEntry::IsRunning()
    {
        return true;
    }
}

