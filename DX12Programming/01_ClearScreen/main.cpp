#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>

#include <Window/Window.h>
#include <D3D12/D3D12AppBase.h>

const wchar_t* APP_NAME = L"01_ClearScreen";

const int WINDOW_WIDTH = 1280; 
const int WINDOW_HEIGHT = 720;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    Window window;
    D3D12AppBase app;

    window.create(
        hInstance,
        nCmdShow,
        APP_NAME,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    return app.Run(window.GetHwnd());
}