#pragma once

#include <stdexcept>

#include "VDeleter.h"
#include "VulkanWindow/VulkanWindow.h"
#include "VulkanDevice/VulkanDeviceManager.h"
#include "VulkanRenderer/VulkanRenderer.h"

class VulkanApplication {
public:
	virtual void Run() = 0;

protected:

	VDeleter<VkInstance> Instance{ vkDestroyInstance };
	VDeleter<VkSurfaceKHR> Surface = { Instance, vkDestroySurfaceKHR };

	VulkanWindow* Window;
	VulkanDeviceManager* DeviceManager;
	VulkanRenderer* Renderer;

	virtual void CreateInstance();
};
