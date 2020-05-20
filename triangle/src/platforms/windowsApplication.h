#pragma once

#include "applicationBase.h"

#include "..\logging\consoleLogger.h"
#include "..\windows\win32Window.h"
#include "..\surface\win32Surface.h"
#include "..\inputs\win32InputManager.h"

#include <thread>

class WindowsApplication : public ApplicationBase
{

public:
	HINSTANCE hInstance;

	virtual void Start(const char * pName) override;
	virtual void HandleError(const char * pMessage) override;
};