#include "Vulkan.h"

int main()
{
    Vulkan::VulkanApplication app;
    app.querryAvailableLayers();
    app.getAvailableInstanceExtensions();
    app.createInstance();
    app.createDebugMesenger();
    app.pickUpPhysicalDevice();
    app.createLogicalDevice();
    app.clean();
}
