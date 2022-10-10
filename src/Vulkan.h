#ifndef VULKAN
#define VULKAN

#include <cstdint>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <array>

#include "Core/Core.h"
#include "vulkan/vulkan.h"
#include "QueueInfo.h"
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

		//pickup physical device
		void pickUpPhysicalDevice();
		bool isDeviceIsSuitable(VkPhysicalDevice device);

		//create logical device
		void createLogicalDevice();

		//cleaning
		void clean();

	private:
		//specifying the list of needed layers and DEVICE extensions
		const std::vector<const char*> usedValidationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		const std::vector<const char*> usedDeviceExtensions = {
			"VK_KHR_swapchain",
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

		//storing physical device and all necessar data
		VkPhysicalDevice physicalDevice;

		//queues
		QueuesInfo info;
		bool computeQueueIndexEqualToGraphicsQueueIndex;
		VkQueue graphicsQueue;
		VkQueue computeQueue;

		//create logical device
		VkDevice device;

	private://STATICS
		//function for creation of VkDebugUtilsMessengerEXT object
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugMesengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			if (messageSeverity >=
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				LOG_INFO("VALIDATION LAYER");
				LOG_INFO(pCallbackData->pMessage);
				LOG_INFO('\n');
			}

			return VK_FALSE;
		}
	};
}

#endif