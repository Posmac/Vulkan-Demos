#include <cstdint>
#include <vector>
#include <string>
#include <set>

#include "Core/Core.h"
#include "vulkan/vulkan.h"

namespace Vulkan
{
	class VulkanApplication
	{
	public:
		VulkanApplication();

		//querry available layers
		void querryAvailableLayers();

		//checking available device extensions
		void getAvailableInstanceExtensions();

		//creating vulkan instance
		void createInstance();

		//create a debug mesenger
		void createDebugMesenger();
		VkDebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerInfo();

		//delete a debug messenger
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
			VkDebugUtilsMessengerEXT debugMessenger, 
			const VkAllocationCallbacks* pAllocator);

		//cleaning
		void clean();

	private:
		//return vector of string for layers and extensions
		void getLayerNames(const std::vector<VkLayerProperties> &originalList,
									std::vector<const char*> &returnList);
		void getExtensionNames(const std::vector<VkExtensionProperties> &originalList,
        								std::vector<const char*> &returnList);

	private:
		//specifying the list of needed layers and DEVICE extensions
		const std::vector<const char*> usedValidationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		const std::vector<const char*> usedDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			"VK_EXT_debug_report",
			"VK_EXT_debug_utils",
		};

		//checking available layers
		uint32_t availableLayersCount;
		//storing available layers
		std::vector<VkLayerProperties> availableLayers;

		//creating a debug mesenger object for layers
		bool debugModeEnabled;
		VkDebugUtilsMessengerEXT debugMessenger;

		//checking available instance extensions
		uint32_t availableInstanceExtensionsCount;
		//storing available instance extensions
		std::vector<VkExtensionProperties> availableInstanceExtensions;

		//storing instance object
		VkInstance instance;

	private://STATICS
		//function for creation of VkDebugUtilsMessengerEXT object
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugMesengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			LOG_INFO("VALIDATION LAYER");
			LOG_INFO(pCallbackData->pMessage);
			LOG_INFO('\n');

			return VK_FALSE;
		}
	};
}