#pragma once

#include "../framework.h"

//! @brief ウィンドウ
class Window
{
public:
	Window();
	~Window();

	//! @brief 作成
	void create(HINSTANCE hInstance, int nCmdShow, const wchar_t* appName, const uint32_t width, const uint32_t height);

	//! @brief ウィンドウハンドル
	HWND GetHwnd() const;

private:
	HWND mHwnd;

	uint32_t mWidth;
	uint32_t mHeight;
};