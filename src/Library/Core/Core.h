#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <sstream>

#include "vulkan/vulkan.h"
#include "vulkan/vk_platform.h"

#include "Library/Common/Log.h"

namespace vk
{
#define LOG_INFO(x) std::cout << (x) << std::endl

#define VK_CHECK_RESULT(f) \
    {																										\
        VkResult res = (f);																					\
        if (res != VK_SUCCESS) {																			\
            std::cout << "Fatal : VkResult is \"" << (res) <<                                               \
            "\" in " << __FILE__ << " at line " << __LINE__ << std::endl;                                   \
            assert(res == VK_SUCCESS);																	    \
        }																									\
    }
}