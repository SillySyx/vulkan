#include "RendererVulkanBase.h"

void RendererVulkanBase::InitiateCommon()
{
	CreateInstance();
	CreateDebugReportCallback();

	GetPhysicalDevice();
	CreateLogicalDevice(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	GetGraphicsQueue();

	GetSupportedDepthFormat();

	// Create a semaphore used to synchronize image presentation
	// Ensures that the image is displayed before we start submitting new commands to the queue
	CreateSynchronizationSemaphore(&semaphorePresentation, "image presentation");

	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands have been sumbitted and executed
	CreateSynchronizationSemaphore(&semaphoreRender, "command submission");

	CreateSubmitInfoStructure();
}

#if defined(_WIN32)
void RendererVulkanBase::InitiateWin32(void* platformHandle, void* platformWindow)
{
	InitiateCommon();

	CreateSurfaceWin32(platformHandle, platformWindow);

	Initiate();
}
#elif defined(__ANDROID__)
void RendererVulkanBase::InitiateAndroid(ANativeWindow* window)
{
	InitiateCommon();

	CreateSurfaceAndroid(window);

	Initiate();
}
#elif defined(__linux__)
void RendererVulkanBase::InitiateLinux(xcb_connection_t* connection, xcb_window_t window)
{
	InitiateCommon();

	CreateSurfaceLinux(connection, window);

	Initiate();
}
#endif

RendererVulkanBase::~RendererVulkanBase()
{
	DestroySurface();

	DestroySynchronizationSemaphore(&semaphorePresentation);
	DestroySynchronizationSemaphore(&semaphoreRender);

	DestroyLogicalDevice();

	DestroyDebugReportCallback();

	DestroyInstance();
}

void RendererVulkanBase::StageBuffer(void * source, VkDeviceSize size, VkBufferUsageFlags flags, VkBuffer * buffer, VkDeviceMemory * memory)
{
	logger.Info("RendererVulkanBase::StageBuffer ...");

	// Static data like vertex and index buffer should be stored on the device memory 
	// for optimal (and fastest) access by the GPU

	// To achieve this we use so-called "staging buffers" :
	// - Create a buffer that's visible to the host (and can be mapped)
	// - Copy the data to this buffer
	// - Create another buffer that's local on the device (VRAM) with the same size
	// - Copy the data from the host to the device using a command buffer
	// - Delete the host visible (staging) buffer
	// - Use the device local buffers for rendering

	VkDeviceMemory stagingMemory;
	VkBuffer stagingBuffer;
	void * data;

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	// Create a host buffer 
	if (vkCreateBuffer(logicalDevice, &createInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
		throw "Failed to create host buffer";

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(logicalDevice, stagingBuffer, &memReqs);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &stagingMemory) != VK_SUCCESS)
		throw "Failed to allocate host buffer memory";

	// Map and copy
	if (vkMapMemory(logicalDevice, stagingMemory, 0, memAlloc.allocationSize, 0, &data) != VK_SUCCESS)
		throw "Failed to map memory";

	memcpy(data, source, size);
	vkUnmapMemory(logicalDevice, stagingMemory);

	if (vkBindBufferMemory(logicalDevice, stagingBuffer, stagingMemory, 0) != VK_SUCCESS)
		throw "Failed to bind host buffer memory";

	// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
	createInfo.usage = flags;
	if (vkCreateBuffer(logicalDevice, &createInfo, nullptr, buffer) != VK_SUCCESS)
		throw "Failed to create device buffer";

	vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(logicalDevice, &memAlloc, nullptr, memory) != VK_SUCCESS)
		throw "Failed to allocate device buffer memory";

	if (vkBindBufferMemory(logicalDevice, *buffer, *memory, 0) != VK_SUCCESS)
		throw "Failed to bind device buffer memory";

	// Buffer copies have to be submitted to a queue, so we need a command buffer for them
	// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer copyCmd;
	if (vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, &copyCmd) != VK_SUCCESS)
		throw "Failed to allocate command buffers";

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;
	if (vkBeginCommandBuffer(copyCmd, &cmdBufInfo) != VK_SUCCESS)
		throw "Failed to begin command buffer";

	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(copyCmd, stagingBuffer, *buffer, 1, &copyRegion);

	// Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
	if (vkEndCommandBuffer(copyCmd) != VK_SUCCESS)
		throw "Failed to end command buffer";

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;

	VkFence fence;
	if (vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		throw "failed to create fence";

	// Submit to the queue
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw "Failed to submit queue";

	// Wait for the fence to signal that command buffer has finished executing
	if (vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, 100000000000) != VK_SUCCESS)
		throw "failed to wait for fences";

	vkDestroyFence(logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &copyCmd);

	// Destroy staging buffers
	// Note: Staging buffer must not be deleted before the copies have been submitted and executed
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingMemory, nullptr);

	logger.Info("RendererVulkanBase::StageBuffer buffer " + logger.GetAddr(&buffer));
	logger.Info("RendererVulkanBase::StageBuffer memory " + logger.GetAddr(&memory));
}

uint32_t RendererVulkanBase::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 * memTypeFound)
{
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

#if defined(__ANDROID__)
	//todo : Exceptions are disabled by default on Android (need to add LOCAL_CPP_FEATURES += exceptions to Android.mk), so for now just return zero
	if (memTypeFound)
	{
		*memTypeFound = false;
	}
	return 0;
#else
	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}
	else
	{
		throw "Could not find a matching memory type";
	}
#endif
}

void RendererVulkanBase::CreateInstance()
{
	logger.Info("RendererVulkanBase::CreateInstance ...");

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "";
	appInfo.pEngineName = "";
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

	// Enable surface extensions depending on os
#if defined(_WIN32)
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
	enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	if (enabledExtensions.size() > 0)
	{
		if (enableValidation)
		{
			enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	if (enableValidation)
	{
		const char *validationLayerNames[] =
		{
			// This is a meta layer that enables all of the standard
			// validation layers in the correct order :
			// threading, parameter_validation, device_limits, object_tracker, image, core_validation, swapchain, and unique_objects
			"VK_LAYER_LUNARG_standard_validation"
		};

		instanceCreateInfo.enabledLayerCount = 1;
		instanceCreateInfo.ppEnabledLayerNames = validationLayerNames;
	}

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
		throw "failed to create instance";

	logger.Info("RendererVulkanBase::CreateInstance " + logger.GetAddr(instance));
}

void RendererVulkanBase::DestroyInstance()
{
	if (instance)
	{
		logger.Info("RendererVulkanBase::DestroyInstance " + logger.GetAddr(instance));

		vkDestroyInstance(instance, nullptr);
		instance = nullptr;

		logger.Info("RendererVulkanBase::DestroyInstance done");
	}
}

void RendererVulkanBase::CreateDebugReportCallback()
{
	logger.Info("RendererVulkanBase::CreateDebugReportCallback ...");

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pfnCallback = DebugMessageCallback;
	dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT; // VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT

	auto createDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (createDebugReportCallbackEXT(instance, &dbgCreateInfo, nullptr, &debugReportCallback) != VK_SUCCESS)
		throw "Failed to create debug report callback";

	logger.Info("RendererVulkanBase::CreateDebugReportCallback " + logger.GetAddr(debugReportCallback));
}

VkBool32 RendererVulkanBase::DebugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData)
{
	LoggerConsole logger;

	// Select prefix depending on flags passed to the callback
	// Note that multiple flags may be set for a single validation message
	std::string message("");

	// Error that may result in undefined behaviour
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		message += "ERROR:";

	// Warnings may hint at unexpected / non-spec API usage
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		message += "WARNING:";

	// May indicate sub-optimal usage of the API
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		message += "PERFORMANCE:";

	// Informal messages that may become handy during debugging
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		message += "INFO:";

	// Diagnostic info from the Vulkan loader and layers
	// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		message += "DEBUG:";

	// Display message to default output (console if activated)
	message += " [" + std::string(pLayerPrefix) + "] Code " + std::to_string(msgCode) + "\n" + pMsg;

	logger.Error(message);

	fflush(stdout);

	// The return value of this callback controls wether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}

void RendererVulkanBase::DestroyDebugReportCallback()
{
	if (enableValidation && debugReportCallback) {
		logger.Info("RendererVulkanBase::DestroyDebugReportCallback " + logger.GetAddr(debugReportCallback));

		auto destroyDebugReportCallbackEX = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (destroyDebugReportCallbackEX)
		{
			destroyDebugReportCallbackEX(instance, debugReportCallback, nullptr);
			debugReportCallback = nullptr;

			logger.Info("RendererVulkanBase::DestroyDebugReportCallback done");
		}
	}
}

void RendererVulkanBase::GetPhysicalDevice()
{
	uint32_t count = 0;
	if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS)
		throw "Failed to get physical devices count";

	logger.Info("RendererVulkanBase::GetPhysicalDevice found " + std::to_string(count) + " device(s)");

	std::vector<VkPhysicalDevice> physicalDevices(count);
	if (vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()) != VK_SUCCESS)
		throw "Failed to get list of physical devices";

	// selects the first one, should check the device capabilities and select the most optimal one
	physicalDevice = physicalDevices[0];

	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	std::string deviceName(physicalDeviceProperties.deviceName);
	logger.Info("RendererVulkanBase::GetPhysicalDevice selected device: " + deviceName);

	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
	physicalDeviceQueueFamilyProperties.resize(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, physicalDeviceQueueFamilyProperties.data());

	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
	if (extCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
		{
			for (auto ext : extensions)
			{
				supportedExtensions.push_back(ext.extensionName);
			}
		}
	}
}

void RendererVulkanBase::CreateLogicalDevice(VkQueueFlags requestedQueueTypes)
{
	logger.Info("RendererVulkanBase::CreateLogicalDevice ...");

	const float defaultQueuePriority(0.0f);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		graphicsQueueFamilyIndex = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	}

	// Dedicated compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
	{
		computeQueueFamilyIndex = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
		if (computeQueueFamilyIndex != graphicsQueueFamilyIndex)
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = computeQueueFamilyIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else
	{
		// Else we use the same queue
		computeQueueFamilyIndex = graphicsQueueFamilyIndex;
	}

	// Dedicated transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
	{
		transferQueueFamilyIndex = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
		if ((transferQueueFamilyIndex != graphicsQueueFamilyIndex) && (transferQueueFamilyIndex != computeQueueFamilyIndex))
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = transferQueueFamilyIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else
	{
		// Else we use the same queue
		transferQueueFamilyIndex = graphicsQueueFamilyIndex;
	}

	std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	if (deviceExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	}

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		throw "Failed to create logical device";

	logger.Info("RendererVulkanBase::CreateLogicalDevice " + logger.GetAddr(logicalDevice));
}

void RendererVulkanBase::DestroyLogicalDevice()
{
	if (logicalDevice)
	{
		logger.Info("RendererVulkanBase::DestroyLogicalDevice " + logger.GetAddr(logicalDevice));

		vkDestroyDevice(logicalDevice, nullptr);
		logicalDevice = nullptr;

		logger.Info("RendererVulkanBase::DestroyLogicalDevice done");
	}
}

uint32_t RendererVulkanBase::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
{
	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags & VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(physicalDeviceQueueFamilyProperties.size()); i++)
		{
			if ((physicalDeviceQueueFamilyProperties[i].queueFlags & queueFlags) &&
				((physicalDeviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				return i;
				break;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (queueFlags & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(physicalDeviceQueueFamilyProperties.size()); i++)
		{
			if ((physicalDeviceQueueFamilyProperties[i].queueFlags & queueFlags) &&
				((physicalDeviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
				((physicalDeviceQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				return i;
				break;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (uint32_t i = 0; i < static_cast<uint32_t>(physicalDeviceQueueFamilyProperties.size()); i++)
	{
		if (physicalDeviceQueueFamilyProperties[i].queueFlags & queueFlags)
		{
			return i;
			break;
		}
	}

	throw "Could not find a matching queue family index";
}

void RendererVulkanBase::GetGraphicsQueue()
{
	logger.Info("RendererVulkanBase::GetGraphicsQueue ...");

	// Get a graphics queue from the device
	vkGetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	if (graphicsQueue == nullptr)
		throw "Failed to get graphics queue";

	logger.Info("RendererVulkanBase::GetGraphicsQueue " + logger.GetAddr(graphicsQueue));
}

void RendererVulkanBase::GetSupportedDepthFormat()
{
	logger.Info("RendererVulkanBase::GetSupportedDepthFormat ...");

	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::map<VkFormat, std::string> formats = {
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, "VK_FORMAT_D32_SFLOAT_S8_UINT" },
		{ VK_FORMAT_D32_SFLOAT, "VK_FORMAT_D32_SFLOAT" },
		{ VK_FORMAT_D24_UNORM_S8_UINT, "VK_FORMAT_D24_UNORM_S8_UINT" },
		{ VK_FORMAT_D16_UNORM_S8_UINT, "VK_FORMAT_D16_UNORM_S8_UINT" },
		{ VK_FORMAT_D16_UNORM, "VK_FORMAT_D16_UNORM" },
	};

	for (const auto& format : formats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format.first, &formatProps);

		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = format.first;

			logger.Info("RendererVulkanBase::GetSupportedDepthFormat " + format.second);
			return;
		}
	}

	throw "Failed to find supported depth format";
}

void RendererVulkanBase::CreateSynchronizationSemaphore(VkSemaphore * semaphore, const char * name)
{
	logger.Info("RendererVulkanBase::CreateSynchronizationSemaphore " + std::string(name) + " ...");

	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;

	if (vkCreateSemaphore(logicalDevice, &createInfo, nullptr, semaphore) != VK_SUCCESS)
		throw "Failed to create semaphore ";

	logger.Info("RendererVulkanBase::CreateSynchronizationSemaphore " + std::string(name) + " " + logger.GetAddr(&semaphore));
}

void RendererVulkanBase::DestroySynchronizationSemaphore(VkSemaphore * semaphore)
{
	if (logicalDevice && *semaphore)
	{
		logger.Info("RendererVulkanBase::DestroySynchronizationSemaphore " + logger.GetAddr(&semaphore));

		vkDestroySemaphore(logicalDevice, *semaphore, nullptr);
		*semaphore = nullptr;

		logger.Info("RendererVulkanBase::DestroySynchronizationSemaphore done");
	}
}

void RendererVulkanBase::CreateSubmitInfoStructure()
{
	logger.Info("RendererVulkanBase::CreateSubmitInfoStructure ...");

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphorePresentation;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphoreRender;

	logger.Info("RendererVulkanBase::CreateSubmitInfoStructure " + logger.GetAddr(&submitInfo));
}

#if defined(_WIN32)
void RendererVulkanBase::CreateSurfaceWin32(void* platformHandle, void* platformWindow)
{
	logger.Info("RendererVulkanBase::CreateSurfaceWin32 ...");

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
	surfaceCreateInfo.hwnd = (HWND)platformWindow;

	if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface))
		throw "failed to create win32 surface";

	logger.Info("RendererVulkanBase::CreateSurfaceWin32 " + logger.GetAddr(surface));

	InitiateSurface();
}
#elif defined(__ANDROID__)
void RendererVulkanBase::CreateSurfaceAndroid(ANativeWindow* window)
{
	logger.Info("RendererVulkanBase::CreateSurfaceAndroid ...");

	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.window = window;

	if (vkCreateAndroidSurface(instance, &surfaceCreateInfo, NULL, &surface))
		throw "failed to create android surface";

	logger.Info("RendererVulkanBase::CreateSurfaceAndroid " + logger.GetAddr(&surface));

	InitiateSurface();
}
#elif defined(__linux__)
void RendererVulkanBase::CreateSurfaceLinux(xcb_connection_t* connection, xcb_window_t window)
{
	logger.Info("RendererVulkanBase::CreateSurfaceLinux ...");

	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.connection = connection;
	surfaceCreateInfo.window = window;

	if (vkCreateXcbSurface(instance, &surfaceCreateInfo, nullptr, &surface))
		throw "failed to create xcb surface";

	logger.Info("RendererVulkanBase::CreateSurfaceLinux " + logger.GetAddr(&surface));

	InitiateSurface();
}
#endif

void RendererVulkanBase::InitiateSurface()
{
	logger.Info("RendererVulkanBase::InitiateSurface ...");

	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);

	std::vector<VkQueueFamilyProperties> queueProps(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

	// Iterate over each queue to learn whether it supports presenting:
	// Find a queue with present support
	// Will be used to present the swap chain images to the windowing system
	std::vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i = 0; i < queueCount; i++)
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == UINT32_MAX)
			{
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// If there's no queue that supports both present and graphics
		// try to find a separate present queue
		for (uint32_t i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Exit if either a graphics or a presenting queue hasn't been found
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		throw "Could not find a graphics and/or presenting queue!";

	if (graphicsQueueNodeIndex != presentQueueNodeIndex)
		throw "Separate graphics and presenting queues are not supported yet!";

	detectedGraphicsQueueIndex = graphicsQueueNodeIndex;

	// Get list of supported surface formats
	uint32_t formatCount;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL) != VK_SUCCESS)
		throw "Failed to get physical device surface formats count";

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()) != VK_SUCCESS)
		throw "Failed to get physical device surface formats data";

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		surfaceColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		// Always select the first available color format
		// If you need a specific format (e.g. SRGB) you'd need to
		// iterate over the list of available surface format and
		// check for it's presence
		surfaceColorFormat = surfaceFormats[0].format;
	}
	surfaceColorSpace = surfaceFormats[0].colorSpace;

	logger.Info("RendererVulkanBase::InitiateSurface " + logger.GetAddr(surface));
}

void RendererVulkanBase::DestroySurface()
{
	if (surface)
	{
		logger.Info("RendererVulkanBase::DestroySurface " + logger.GetAddr(surface));

		vkDestroySurfaceKHR(instance, surface, nullptr);
		surface = nullptr;

		logger.Info("RendererVulkanBase::DestroySurface done");
	}
}