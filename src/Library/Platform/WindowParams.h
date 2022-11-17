#pragma once

#include <Windows.h>
#include "vulkan/vulkan_win32.h"

namespace vk
{
    struct WindowParameters
    {
        HINSTANCE hInstance;
        HWND hWnd;
    };
}