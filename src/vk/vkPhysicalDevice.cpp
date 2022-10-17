#include "vkPhysicalDevice.h"

namespace vk
{
	CVkPhysicalDevice::CVkPhysicalDevice()
	{

	}



	VkPhysicalDevice CVkPhysicalDevice::GetGPU() const
	{
		return physicalDevice;
	}

	bool CVkPhysicalDevice::IsQueuesIdentic() const
	{
		return computeQueueIndexEqualToGraphicsQueueIndex;
	}

	QueuesInfo CVkPhysicalDevice::GetQueuesInfo() const
	{
		return info;
	}

	VkQueue CVkPhysicalDevice::GetGraphicsQueue()
	{
		return graphicsQueue;
	}

	VkQueue CVkPhysicalDevice::GetComputeQueue()
	{
		return computeQueue;
	}

	void CVkPhysicalDevice::PickUpPhysicalDevice(const VkInstance& instance)
	{
		uint32_t availablePhysicalDevices = 0;
		vkEnumeratePhysicalDevices(instance, &availablePhysicalDevices, nullptr);

		if (availablePhysicalDevices == 0)
		{
			LOG_INFO("Failed to querry any physical device on system");
		}

		std::vector<VkPhysicalDevice> devices(availablePhysicalDevices);
		vkEnumeratePhysicalDevices(instance, &availablePhysicalDevices, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceIsSuitable(device))
			{
				physicalDevice = device;
				return;
			}
		}
		LOG_INFO("Failed to find suitable physical devices, picked first one");
		physicalDevice = devices[0];
	}

	bool CVkPhysicalDevice::IsDeviceIsSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceProperties props;

		uint32_t queueFamilies;
		std::vector<VkQueueFamilyProperties> familyProps;

		vkGetPhysicalDeviceFeatures(device, &features);
		vkGetPhysicalDeviceProperties(device, &props);

		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilies, nullptr);
		familyProps.resize(queueFamilies);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilies, familyProps.data());

		uint32_t graphicsFamily;
		uint32_t computeFamily;

		info = {};

		computeQueueIndexEqualToGraphicsQueueIndex = false;

		for (int i = 0; i < queueFamilies; i++)
		{
			if (familyProps[i].queueCount > 0 && features.geometryShader)
			{
				if ((familyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
				{
					info.computeFamily = i;
				}

				if ((familyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					info.graphicsFamily = i;
				}

				if (info.isComplete())
				{
					if (info.graphicsFamily.value() == info.computeFamily.value())
					{
						computeQueueIndexEqualToGraphicsQueueIndex = true;
					}
					return true;
				}
			}
		}

		LOG_INFO("Cannot find suitable physical device");

		return false;
	}
}
