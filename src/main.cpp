#include "Vulkan.h"

int main()
{
    Vulkan::VulkanApplication app;

    //creating instance with layers and extensions
    app.querryAvailableLayers();
    app.getAvailableInstanceExtensions();
    app.createInstance();

    //creating debug messenger to log validation layers info
    app.createDebugMesenger();

    //creating logical device
    app.pickUpPhysicalDevice();
    app.createLogicalDevice();

    //window system creation
    app.createSurface();
    
    app.clean();
}
