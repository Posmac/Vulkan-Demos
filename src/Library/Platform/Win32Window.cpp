#include "Win32Window.h"

namespace vk
{
    Win32Window::Win32Window()
    {

    }

    WindowParameters Win32Window::GetWindowParams() const
    {
        return params;
    }

    void Win32Window::CreateWindowsWindow(int width, int height)
    {
        params.hInstance = GetModuleHandle(NULL);

        auto size = sizeof(WNDCLASSEX);
        const char* className = "Vulkan Demo";

        WNDCLASSEX wndClass{};
        wndClass.cbSize = size;
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = params.hInstance;
        wndClass.hIcon = NULL;
        wndClass.hCursor = LoadCursor(0, IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
        wndClass.lpszMenuName = "Menu";
        wndClass.lpszClassName = className;
        wndClass.hIcon = LoadIcon(0, IDI_WINLOGO);
        wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.lpfnWndProc = WindowProcedure;

        if (!RegisterClassEx(&wndClass))
        {
            LOG_INFO("Failed to register class");
        }

        params.hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, className, "Vulkan", WS_OVERLAPPEDWINDOW,
            0, 0, width, height, NULL, NULL, params.hInstance, NULL);

        if (params.hWnd == nullptr)
        {
            LOG_INFO("Failed to create window handle");
        }
    }

    void Win32Window::Render(Sample &sample)
    {
        ShowWindow(params.hWnd, SW_SHOWNORMAL);
        UpdateWindow(params.hWnd);
        MSG message;
        bool loop = true;

        while (loop)
        {
            if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
            {
                switch (message.message)
                {
                case USER_MESSAGE_QUIT:
                    loop = false;
                    break;
                }
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                if (sample.IsReady())
                {
                    sample.Draw();
                }
            }
        }
    }

    LRESULT CALLBACK Win32Window::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_LBUTTONDOWN:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 1);
            break;
        case WM_LBUTTONUP:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 0);
            break;
        case WM_RBUTTONDOWN:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 1);
            break;
        case WM_RBUTTONUP:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 0);
            break;
        case WM_MOUSEMOVE:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_MOVE, LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOUSEWHEEL:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_WHEEL, HIWORD(wParam), 0);
            break;
        case WM_SIZE:
        case WM_EXITSIZEMOVE:
            PostMessage(hWnd, USER_MESSAGE_RESIZE, wParam, lParam);
            break;
        case WM_KEYDOWN:
            if (VK_ESCAPE == wParam) 
            {
                PostMessage(hWnd, USER_MESSAGE_QUIT, wParam, lParam);
            }
            break;
        case WM_CLOSE:
            PostMessage(hWnd, USER_MESSAGE_QUIT, wParam, lParam);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
}
