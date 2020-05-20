#include "win32Window.h"

Win32Window * Win32Window::Create(Win32CreateInfo * pCreateInfo)
{
	auto window = new Win32Window();
	window->pLogger = pCreateInfo->pLogger;
	window->width = pCreateInfo->width;
	window->height = pCreateInfo->height;
	window->fullscreen = pCreateInfo->fullscreen;
	window->hInstance = pCreateInfo->hInstance;
	window->windowResized = pCreateInfo->windowResized;
	window->windowActivation = pCreateInfo->windowActivation;

	window->Create(pCreateInfo->pName);

	return window;
}

void Win32Window::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		pLogger->Info("Win32Window closed");

		DestroyWindow(hWnd);
		PostQuitMessage(0);
		shutdown = true;
		break;

	case WM_PAINT:
		ValidateRect(handle, NULL);
		break;

	case WM_SIZE:
		if (resizing)
		{
			width = LOWORD(lParam);
			height = HIWORD(lParam);
		}
		else if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		{
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			windowResized(width, height);
		}
		break;

	case WM_ENTERSIZEMOVE:
		resizing = true;
		break;

	case WM_EXITSIZEMOVE:
		if (resizing)
		{
			resizing = false;
			windowResized(width, height);
		}
		break;

	case WM_ACTIVATE:
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			windowActivation(true);
		else if (wParam == WA_INACTIVE)
			windowActivation(false);
		break;
	}
}

void Win32Window::Create(const char * pName)
{
	auto winProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (window)
		{
			window->HandleMessage(hwnd, msg, wParam, lParam);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	};

	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = winProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = pName;
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		throw "Could not register window class!";
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		DEVMODE dmScreenSettings = {};
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

	DWORD dwExStyle = {};
	DWORD dwStyle = {};

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

	RECT windowRect = {};
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = fullscreen ? (long)screenWidth : (long)width;
	windowRect.bottom = fullscreen ? (long)screenHeight : (long)height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	handle = CreateWindowEx(
		0,
		pName,
		pName,
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
		SetWindowPos(handle, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if (!handle)
	{
		throw "Could not create window!";
	}

	SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(handle, SW_SHOW);
	SetForegroundWindow(handle);
	SetFocus(handle);

	pLogger->Info("Win32Window created");
}

void Win32Window::StartProcessingMessages()
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

void * Win32Window::GetHandle()
{
	return handle;
}