#pragma once

#include "baseSurface.h"

#include <Windows.h>

struct Win32SurfaceCreateInfo
{
	BaseLogger * pLogger;
	DeviceManager * pDevice;
	HINSTANCE hInstance;
	HWND hwnd;
};

class Win32Surface : public BaseSurface
{
public:

	void Initialize(HINSTANCE hInstance, HWND hwnd);

	static Win32Surface * Create(Win32SurfaceCreateInfo * pCreateInfo);
};