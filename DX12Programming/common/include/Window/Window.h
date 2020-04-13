#pragma once

#include "../framework.h"

//! @brief �E�B���h�E
class Window
{
public:
	Window();
	~Window();

	//! @brief �쐬
	void create(HINSTANCE hInstance, int nCmdShow, const wchar_t* appName, const uint32_t width, const uint32_t height);

	//! @brief �E�B���h�E�n���h��
	HWND GetHwnd() const;

private:
	HWND mHwnd;

	uint32_t mWidth;
	uint32_t mHeight;
};