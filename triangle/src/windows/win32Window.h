#pragma once

#include "baseWindow.h"

#include <windows.h>
#include <functional>

struct Win32CreateInfo {
	BaseLogger * pLogger;
	const char * pName;
	unsigned int width;
	unsigned int height;
	bool fullscreen;
	HINSTANCE hInstance;
	std::function<void(unsigned int, unsigned int)> windowResized;
	std::function<void(bool)> windowActivation;
};

class Win32Window : public BaseWindow
{
public:
	HWND handle;
	HINSTANCE hInstance;

	std::function<void(unsigned int, unsigned int)> windowResized;
	std::function<void(bool)> windowActivation;

	void HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Create(const char * pName);

	void StartProcessingMessages() override;

	void * GetHandle() override;

	static Win32Window * Create(Win32CreateInfo * pCreateInfo);
};