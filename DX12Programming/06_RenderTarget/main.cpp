﻿#include <stdexcept>

#include <core/windows/myWindows.h>
#include <Window/Window.h>
#include "MyApp.h"

const wchar_t* APP_NAME = L"06_RenderTarget";

const int WINDOW_WIDTH = 1280; 
const int WINDOW_HEIGHT = 720;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    Window window;
    MyApp app;

    window.create(
        hInstance,
        nCmdShow,
        APP_NAME,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    return app.Run(window.GetHwnd());
}