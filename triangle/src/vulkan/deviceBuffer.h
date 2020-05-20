#pragma once

#include "..\logging\baseLogger.h"
#include "..\vulkan\deviceManager.h"

class DeviceBuffer
{
public:
	BaseLogger * pLogger;
	DeviceManager * pDevice;

	~DeviceBuffer();

	void StageBuffer(void * source, VkDeviceSize size, VkBufferUsageFlags flags, VkCommandPool commandPool, VkQueue queue);

protected:
	VkBuffer buffer;
	VkDeviceMemory memory;
};