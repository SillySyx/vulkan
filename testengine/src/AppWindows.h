#pragma once
#pragma comment(linker, "/subsystem:windows")

#include <windows.h>
#include <iostream>
#include <io.h>
#include <string>
#include <chrono>
#include <memory>
#include <thread>

#include "AppBase.h"

#include "Logging/LoggerConsole.h"
#include "Base/SceneManager.h"

class WindowsAppBase : public AppBase
{

public:
	HINSTANCE hInstance;

	~WindowsAppBase();

	virtual void Start() override;
	virtual void HandleError(const char * message) override;

	virtual void WindowResized() override;

	void HandleWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:

	HWND windowHandle;

	LoggerConsole logger;

	RendererForward renderer;
	SceneManager sceneManager;

	bool shutdown = false;
	bool canRender = false;
	bool windowActive = true;
	bool windowResizing = false;

	uint32_t width = 1028;
	uint32_t height = 720;

	double frameMS = 0;
	double updateMS = 0;

	void SetupConsole();
	void SetupWindow();
	std::string GetWindowTitle();

	void MainLoop();
	void RenderLoop();
	void UpdateLoop();
};