#include "DebugMesenger.h"

namespace vk
{
    void GetDebugUtilsMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
    {
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.pNext = nullptr;
        debugCreateInfo.flags = 0;//everytime 0, reserved for later use
        debugCreateInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        debugCreateInfo.pfnUserCallback = DebugMesengerCallback;
        debugCreateInfo.pUserData = nullptr;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    bool CreateDebugMesenger(const VkInstance& instance,
        bool debugModeEnabled,
        VkDebugUtilsMessengerEXT& debugMessenger)
    {
        if (!debugModeEnabled)
        {
            INFO_LOG("Debug mode disabled, no debug object will be created");
            return true;
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        GetDebugUtilsMessengerInfo(debugCreateInfo);

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        auto result = func(instance, &debugCreateInfo, nullptr, &debugMessenger);

        if (result != VK_SUCCESS)
        {
            ERROR_LOG("Failed to create debug utils messenger");
            return false;
        }
        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugMesengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        WARN_LOG("VALIDATION LAYER: " + std::string(pCallbackData->pMessage) + '\n');
        return VK_FALSE;
    }
}