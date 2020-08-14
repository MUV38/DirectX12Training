#include <stdexcept>

#include <core/windows/myWindows.h>
#include <Window/Window.h>
#include <Application/Application.h>

const wchar_t* APP_NAME = L"13_IndirectDraw";

const int WINDOW_WIDTH = 1280; 
const int WINDOW_HEIGHT = 720;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    Window window;
    Application app;

    window.create(
        hInstance,
        nCmdShow,
        APP_NAME,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    return app.Run(window.GetHwnd());
}