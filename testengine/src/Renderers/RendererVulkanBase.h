#pragma once

#include <vector>
#include <map>
#include <string>

#include "RendererBase.h"
#include "../Logging/LoggerConsole.h"
#include "../Vulkan/VulkanWrapper.h"

class RendererVulkanBase : public RendererBase
{

public:

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice logicalDevice = VK_NULL_HANDLE;

	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties;
	std::vector<std::string> supportedExtensions;

	uint32_t detectedGraphicsQueueIndex = UINT32_MAX;
	uint32_t graphicsQueueFamilyIndex = VK_NULL_HANDLE;
	uint32_t computeQueueFamilyIndex = VK_NULL_HANDLE;
	uint32_t transferQueueFamilyIndex = VK_NULL_HANDLE;

	bool enableValidation = true;
	VkDebugReportCallbackEXT debugReportCallback = VK_NULL_HANDLE;

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkFormat surfaceColorFormat;
	VkColorSpaceKHR surfaceColorSpace;

	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkFormat depthFormat;

	VkSemaphore semaphorePresentation = VK_NULL_HANDLE;
	VkSemaphore semaphoreRender = VK_NULL_HANDLE;

	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo;

	VkCommandPool commandPool = VK_NULL_HANDLE;

#if defined(_WIN32)
	void InitiateWin32(void* platformHandle, void* platformWindow);
#elif defined(__ANDROID__)
	void InitiateAndroid(ANativeWindow* window);
#elif defined(__linux__)
	void InitiateLinux(xcb_connection_t* connection, xcb_window_t window);
#endif

	~RendererVulkanBase();
	
	uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr);

	void StageBuffer(void * source, VkDeviceSize size, VkBufferUsageFlags flags, VkBuffer * buffer, VkDeviceMemory * memory);

protected:

	LoggerConsole logger;

	void CreateInstance();
	void DestroyInstance();

	void CreateDebugReportCallback();
	static VkBool32 DebugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData);
	void DestroyDebugReportCallback();

	void GetPhysicalDevice();
	void CreateLogicalDevice(VkQueueFlags requestedQueueTypes);
	void DestroyLogicalDevice();

	uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);

	void GetGraphicsQueue();
	void GetSupportedDepthFormat();

	void CreateSynchronizationSemaphore(VkSemaphore * semaphore, const char * name);
	void DestroySynchronizationSemaphore(VkSemaphore * semaphore);

	void CreateSubmitInfoStructure();

	void InitiateCommon();
#if defined(_WIN32)
	void CreateSurfaceWin32(void* platformHandle, void* platformWindow);
#elif defined(__ANDROID__)
	void CreateSurfaceAndroid(ANativeWindow* window);
#elif defined(__linux__)
	void CreateSurfaceLinux(xcb_connection_t* connection, xcb_window_t window);
#endif

	void InitiateSurface();
	void DestroySurface();
};