#pragma once

#include "Library/Core/Core.h"

namespace vk
{
    bool CreateDebugMesenger(const VkInstance& instance,
        bool debugModeEnabled,
        VkDebugUtilsMessengerEXT& debugMessenger);

    void GetDebugUtilsMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator);

    //function for creation of VkDebugUtilsMessengerEXT object
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMesengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData);
}