#include "AppWindows.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowsAppBase* app = reinterpret_cast<WindowsAppBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (app)
	{
		app->HandleWindowMessage(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	InitVulkanLibrary();
	WindowsAppBase app;

	try
	{
		app.hInstance = hInstance;
		app.Start();
	}
	catch (const char * message)
	{
		app.HandleError(message);
	}

	return 0;
}

WindowsAppBase::~WindowsAppBase()
{
	fflush(stdout);
}

void WindowsAppBase::Start()
{
	SetupConsole();
	SetupWindow();

	renderer.width = width;
	renderer.height = height;

	renderer.InitiateWin32(hInstance, windowHandle);
	sceneManager.Initiate(&renderer);

	canRender = true;

	std::thread renderThread([=] {
		RenderLoop();
	});
	std::thread updateThread([=] {
		UpdateLoop();
	});
	MainLoop();
	renderThread.join();
	updateThread.join();
}

void WindowsAppBase::HandleError(const char * message)
{
	logger.Error(message);
}

void WindowsAppBase::WindowResized()
{
	if (width != renderer.width || height != renderer.height)
	{
		canRender = false;
		sceneManager.ResizeResources(width, height);
		canRender = true;
	}
}

void WindowsAppBase::SetupConsole()
{
	auto title = std::string(name) + std::string(" - Debug console");

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	SetConsoleTitle(TEXT(title.c_str()));
}

void WindowsAppBase::SetupWindow()
{
	logger.Info("WindowsAppBase::SetupWindow ...");

	WNDCLASSEX wndClass;

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = name;
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		throw "Could not register window class!";
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((width != screenWidth) && (height != screenHeight))
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					fullscreen = FALSE;
				}
			}
		}
	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = fullscreen ? (long)screenWidth : (long)width;
	windowRect.bottom = fullscreen ? (long)screenHeight : (long)height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	auto title = GetWindowTitle();
	windowHandle = CreateWindowEx(
		0,
		name,
		title.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(windowHandle, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if (!windowHandle)
	{
		throw "Could not create window!";
	}

	SetWindowLongPtr(windowHandle, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle);
	SetFocus(windowHandle);

	logger.Info("WindowsAppBase::SetupWindow " + logger.GetAddr(&windowHandle));
}

std::string WindowsAppBase::GetWindowTitle()
{
	return std::string(name) + std::string(" - " + std::to_string(width) + "x" + std::to_string(height));
}

void WindowsAppBase::HandleWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		logger.Info("Window closed shutting down");

		DestroyWindow(hWnd);
		PostQuitMessage(0);
		shutdown = true;
		break;

	case WM_PAINT:
		ValidateRect(windowHandle, NULL);
		break;

	case WM_SIZE:
		if (canRender)
		{
			if (windowResizing)
			{
				width = LOWORD(lParam);
				height = HIWORD(lParam);
			}
			else if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
			{
				width = LOWORD(lParam);
				height = HIWORD(lParam);
				WindowResized();
			}
		}
		break;

	case WM_ENTERSIZEMOVE:
		windowResizing = true;
		break;

	case WM_EXITSIZEMOVE:
		if (windowResizing)
		{
			windowResizing = false;
			WindowResized();
		}
		break;

	case WM_ACTIVATE:
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			windowActive = true;
		else if (wParam == WA_INACTIVE)
			windowActive = false;
		break;
	}
}

void WindowsAppBase::MainLoop()
{
	MSG msg;
	while (!shutdown)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void WindowsAppBase::RenderLoop()
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;

	double timeToSleep = 0;
	double renderMS = 0;

	auto activeFrequency = 1000. / 144;
	auto inactiveFrequency = 1000. / 15;

	while (!shutdown)
	{
		if (canRender && !windowResizing)
		{
			start = std::chrono::high_resolution_clock::now();

			sceneManager.Render();

			end = std::chrono::high_resolution_clock::now();
			renderMS = std::chrono::duration<double, std::milli>(end - start).count();

			timeToSleep = (windowActive ? activeFrequency : inactiveFrequency) - renderMS;
			if (timeToSleep > 0)
				std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(timeToSleep));

			end = std::chrono::high_resolution_clock::now();
			frameMS = std::chrono::duration<double, std::milli>(end - start).count();
		}
	}
}

void WindowsAppBase::UpdateLoop()
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;

	double timeToSleep = 0;

	auto activeFrequency = 1000. / 10;
	auto inactiveFrequency = 1000. / 1;

	while (!shutdown)
	{
		if (canRender && !windowResizing)
		{
			start = std::chrono::high_resolution_clock::now();

			auto title = GetWindowTitle();

			if (frameMS > 0)
				title += " - frame " + std::to_string(frameMS) + "ms - fps " + std::to_string((int)std::ceil(1000 / frameMS));

			if (updateMS > 0)
				title += " - update " + std::to_string(updateMS) + "ms";

			SetWindowText(windowHandle, title.c_str());

			sceneManager.Update(updateMS);

			end = std::chrono::high_resolution_clock::now();
			updateMS = std::chrono::duration<double, std::milli>(end - start).count();

			timeToSleep = (windowActive ? activeFrequency : inactiveFrequency) - updateMS;
			if (timeToSleep > 0)
				std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(timeToSleep));

			end = std::chrono::high_resolution_clock::now();
			updateMS = std::chrono::duration<double, std::milli>(end - start).count();
		}
	}
}
