#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <stdio.h>

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>

class DeviceManager
{

public:

	VkInstance instance;

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties;

	std::vector<std::string> supportedExtensions;

	VkDevice logicalDevice;

	VkCommandPool commandPool = VK_NULL_HANDLE;

	struct
	{
		uint32_t graphics = VK_NULL_HANDLE;
		uint32_t compute = VK_NULL_HANDLE;
		uint32_t transfer = VK_NULL_HANDLE;
	} queueFamilyIndices;

	void Initialize(char* applicationName, char* engineName, bool enableValidation = false, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);

	~DeviceManager();

protected:

	bool validationEnabled = false;

	VkDebugReportCallbackEXT debugReportCallback;

	VkResult CreateInstance(char* applicationName, char* engineName);
	VkResult CreateDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack);
	VkResult GetPhysicalDevice();
	VkResult CreateLogicalDevice(VkQueueFlags requestedQueueTypes);

	uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);

	static VkBool32 DebugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);
};