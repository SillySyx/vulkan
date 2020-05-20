#pragma once

#include "..\logging\baseLogger.h"
#include "..\vulkan\deviceManager.h"

class BaseSurface
{
public:
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;

	BaseLogger * pLogger;
	DeviceManager * pDevice;
};