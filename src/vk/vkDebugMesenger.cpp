#include "vkDebugMesenger.h"

namespace vk
{
    CVkDebugMessenger::CVkDebugMessenger() { }

    void CVkDebugMessenger::Destroy(const VkInstance &instance)
    {
        if(_debugMessenger != VK_NULL_HANDLE)
        {
            DestroyDebugUtilsMessengerEXT(instance, _debugMessenger, nullptr);
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT CVkDebugMessenger::GetDebugUtilsMessengerInfo() const
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.pNext = nullptr;
        debugInfo.flags = 0;//everytime 0, reserved for later use
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        debugInfo.pfnUserCallback = DebugMesengerCallback;
        debugInfo.pUserData = nullptr;

        return debugInfo;
    }

    void CVkDebugMessenger::DestroyDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void CVkDebugMessenger::CreateDebugMesenger(const VkInstance& instance, bool debugModeEnabled)
    {
       if(!debugModeEnabled)
       {
           LOG_INFO("Debug mode disabled, no debug object will be created");
           return;
       }
       auto debugInfo = GetDebugUtilsMessengerInfo();
       auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
       auto result = func(instance, &debugInfo, nullptr, &_debugMessenger);
       if (result != VK_SUCCESS)
       {
           LOG_INFO("failed to create debug utils messenger");
       }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL CVkDebugMessenger::DebugMesengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		if (messageSeverity >=
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_INFO(" VALIDATION LAYER" + std::string(pCallbackData->pMessage));
		}
		return VK_FALSE;
	}
}