#pragma once

#include "Core.h"

namespace vk
{
    class DebugMessenger
    {
    private:
        	VkDebugUtilsMessengerEXT _debugMessenger;
    private:
    //function for creation of VkDebugUtilsMessengerEXT object
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMesengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    public:
        DebugMessenger();
        void CreateDebugMesenger(const VkInstance& instance, bool debugModeEnabled);
        void Destroy(const VkInstance&instance);

        VkDebugUtilsMessengerCreateInfoEXT GetDebugUtilsMessengerInfo() const;
        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    };
}