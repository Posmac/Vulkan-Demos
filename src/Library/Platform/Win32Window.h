#pragma once

#include "Library/Core/Core.h"
#include "VulkanSample.h"
#include "WindowParams.h"
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

    class Win32Window
    {
    public:
        Win32Window();
        WindowParameters GetWindowParams() const;
        void CreateWindowsWindow(int width, int height);
        void Render(Sample& sample);
    private:
        static LRESULT CALLBACK WindowProcedure( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    private:
        WindowParameters params;
    };
}
