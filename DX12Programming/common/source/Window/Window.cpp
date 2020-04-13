#include "Window/Window.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>

// ImGuiDx12のメッセージ処理
extern LRESULT ImGuiDx12_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // ImGuiDx12のメッセージ処理
    if (ImGuiDx12_WndProcHandler(hWnd, msg, wp, lp))
    {
        return true;
    }

    PAINTSTRUCT ps;
    HDC hdc;
    switch (msg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wp, lp);
}

Window::Window()
    : mHwnd(0)
    , mWidth(1280)
    , mHeight(720)
{
}

Window::~Window()
{
}

// 作成
void Window::create(HINSTANCE hInstance, int nCmdShow, const wchar_t* appName, const uint32_t width, const uint32_t height)
{
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = appName;
    RegisterClassEx(&wc);

    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
    RECT rect = { 0,0, width, height };
    AdjustWindowRect(&rect, dwStyle, FALSE);

    HWND hwnd = CreateWindow(wc.lpszClassName, appName,
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    //SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));
    ShowWindow(hwnd, nCmdShow);

    mHwnd = hwnd;
    mWidth = width;
    mHeight = height;
}

// ウィンドウハンドル
HWND Window::GetHwnd() const
{
    return mHwnd;
}
