#pragma once

#include "..\logging\baseLogger.h"

#include "functionPointers.h"

#include <string>
#include <vector>

struct DeviceManagerCreateInfo
{
	BaseLogger * pLogger;
	bool enableValidation;
	const char * surfaceExtension;
};

class DeviceManager
{
public:
	BaseLogger * pLogger;
	bool enableValidation = false;

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice logicalDevice = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT debugReportCallback = VK_NULL_HANDLE;

	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties;
	std::vector<std::string> supportedExtensions;

	unsigned int graphicsQueueFamilyIndex = VK_NULL_HANDLE;
	unsigned int computeQueueFamilyIndex = VK_NULL_HANDLE;
	unsigned int transferQueueFamilyIndex = VK_NULL_HANDLE;

	~DeviceManager();

	void Initialize(const char * surfaceExtension);

	void CreateInstance(const char * surfaceExtension);
	void CreateDebugReportCallback();
	void GetPhysicalDevice();
	void CreateLogicalDevice(VkQueueFlags requestedQueueTypes);
	unsigned int GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
	unsigned int GetMemoryType(unsigned int typeBits, VkMemoryPropertyFlags properties, VkBool32 * memTypeFound = nullptr);

	static DeviceManager * Create(DeviceManagerCreateInfo * pCreateInfo);
};