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
#include "Platform/Window.h"

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
///////////////////////////////////////////////////////////////////////////

		void createSurface();

    	enum UserMessage {
    	  USER_MESSAGE_RESIZE = WM_USER + 1,
    	  USER_MESSAGE_QUIT,
    	  USER_MESSAGE_MOUSE_CLICK,
    	  USER_MESSAGE_MOUSE_MOVE,
    	  USER_MESSAGE_MOUSE_WHEEL
    	};

 		static LRESULT CALLBACK WindowProcedure( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
		{
			switch( message ) 
			{
    			case WM_LBUTTONDOWN:
    			  PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 1 );
    			  break;
    			case WM_LBUTTONUP:
    			  PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 0 );
    			  break;
    			case WM_RBUTTONDOWN:
    			  PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 1 );
    			  break;
    			case WM_RBUTTONUP:
    			  PostMessage( hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 0 );
    			  break;
    			case WM_MOUSEMOVE:
    			  PostMessage( hWnd, USER_MESSAGE_MOUSE_MOVE, LOWORD( lParam ), HIWORD( lParam ) );
    			  break;
    			case WM_MOUSEWHEEL:
    			  PostMessage( hWnd, USER_MESSAGE_MOUSE_WHEEL, HIWORD( wParam ), 0 );
    			  break;
    			case WM_SIZE:
    			case WM_EXITSIZEMOVE:
    			  PostMessage( hWnd, USER_MESSAGE_RESIZE, wParam, lParam );
    			  break;
    			case WM_KEYDOWN:
    			  if( VK_ESCAPE == wParam ) {
    			    PostMessage( hWnd, USER_MESSAGE_QUIT, wParam, lParam );
    			  }
    			  break;
    			case WM_CLOSE:
    			  PostMessage( hWnd, USER_MESSAGE_QUIT, wParam, lParam );
    			  break;
    			default:
    			  return DefWindowProc( hWnd, message, wParam, lParam );
    		}
    		return 0;
		}

		//cleaning
		void clean();

	private:
		//specifying the list of needed layers and extensions(for instance and device creation)
		const std::vector<const char*> usedValidationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		const std::vector<const char*> usedInstanceExtensions = 
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
			"VK_EXT_debug_report",
			"VK_EXT_debug_utils"
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

///////////////////////////////////////////////////////////////////
		
		//surface
		VkSurfaceKHR surface;

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
				LOG_INFO(" VALIDATION LAYER" + std::string(pCallbackData->pMessage));
			}

			return VK_FALSE;
		}
	};
}

#endif