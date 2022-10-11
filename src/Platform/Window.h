#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vk_platform.h"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <strsafe.h>

namespace Vulkan
{
    struct WindowParametes
    {
        HINSTANCE hInstance;
        HWND hWnd;
    };
}
