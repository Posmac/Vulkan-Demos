#pragma once

#include "Core.h"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#include <strsafe.h>

namespace vk
{
    enum UserMessage 
    {
        USER_MESSAGE_RESIZE = WM_USER + 1,
        USER_MESSAGE_QUIT,
        USER_MESSAGE_MOUSE_CLICK,
        USER_MESSAGE_MOUSE_MOVE,
        USER_MESSAGE_MOUSE_WHEEL
    };

    struct WindowParameters
    {
        HINSTANCE hInstance;
        HWND hWnd;
    };

    class Win32Window
    {
    public:
        Win32Window();
        WindowParameters GetWindowParams() const;
        void CreateWindowsWindow(int width, int height);
    private:
        static LRESULT CALLBACK WindowProcedure( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    private:
        WindowParameters params;
    };
}
