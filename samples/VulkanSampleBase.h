#pragma once

#include "Library/Core/Core.h"
#include "Library/Platform/WindowParams.h"
#include "Library/Source/Instance.h"
#include "Library/Source/DebugMesenger.h"
#include "Library/Source/LogicalDevice.h"
#include "Library/Source/PhysicalDevice.h"
#include "Library/Source/Swapchain.h"
#include "Library/Source/CommandBuffer.h"
#include "Library/Source/Resources.h"
#include "Library/Source/RenderPass.h"
#include "Library/Source/Pipeline.h"
#include "Library/Source/Drawing.h"
#include "Library/Source/DescriptorSets.h"

namespace vk
{
	class SampleBase
	{
	public:
		inline SampleBase() : isReady(false) {};
		inline virtual ~SampleBase() {};

		virtual bool Initialize(WindowParameters& windowParams, 
			std::vector<const char*> validationLayer,
			std::vector<const char*> instanceExtensions,
			std::vector<const char*> deviceExtensions) = 0;

		virtual bool Draw() = 0;
		virtual bool Resize() = 0;
		virtual void Destroy() = 0;

		inline virtual bool IsReady() { return isReady; };
	protected:
		bool isReady;
	};


}