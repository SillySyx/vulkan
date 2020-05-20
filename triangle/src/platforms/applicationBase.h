#pragma once

#include "..\logging\baseLogger.h"
#include "..\windows\baseWindow.h"
#include "..\vulkan\deviceManager.h"
#include "..\surface\baseSurface.h"
#include "..\renderers\simpleRenderer.h"
#include "..\inputs\baseInputManager.h"
#include "..\scene\sceneManager.h"

#include <string>

class ApplicationBase
{
public:

	bool fullscreen = false;

	virtual void Start(const char * pName) = 0;
	virtual void HandleError(const char * pMessage) = 0;

protected:

	BaseLogger * pLogger;
	BaseWindow * pWindow;
	DeviceManager * pDevice;
	BaseSurface * pSurface;
	SimpleRenderer * pRenderer;

	BaseInputManager * pInputsManager;
	SceneManager sceneManager;
};