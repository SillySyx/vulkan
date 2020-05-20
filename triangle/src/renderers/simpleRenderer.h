#pragma once

#include "..\logging\baseLogger.h"
#include "..\vulkan\deviceManager.h"
#include "..\surface\baseSurface.h"

struct SimpleRendererCreateInfo
{
	BaseLogger * pLogger;
	DeviceManager * pDevice;
	BaseSurface * pSurface;
};

class SimpleRenderer
{
public:
	BaseLogger * pLogger;
	DeviceManager * pDevice;
	BaseSurface * pSurface;

	VkQueue graphicsQueue = VK_NULL_HANDLE;
	unsigned int detectedGraphicsQueueIndex = UINT32_MAX;

	void Initialize();

	void GetGraphicsQueue();

	static SimpleRenderer * Create(SimpleRendererCreateInfo * pCreateInfo);
};