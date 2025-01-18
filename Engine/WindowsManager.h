#pragma once
#include <Windows.h>
#include "Vector2.h"

#include <functional>

class WindowsManager
{
public:
	HRESULT CreateDesktopWindow();
	HWND GetWindowHandle() const;
	HINSTANCE GetWinInstance() const;

	Math::Vector2 GetWindowSize();

	static void CreateWindowsLoop(const std::function<void()>& functionBody);

private:
	static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static HINSTANCE gInstance;
	static HWND gWindowHandle;
};

