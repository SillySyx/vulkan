#include "deviceManager.h"

BaseLogger * globalLogger;

DeviceManager * DeviceManager::Create(DeviceManagerCreateInfo * pCreateInfo)
{
	auto deviceManager = new DeviceManager;
	deviceManager->pLogger = pCreateInfo->pLogger;
	deviceManager->enableValidation = pCreateInfo->enableValidation;

	globalLogger = pCreateInfo->pLogger;

	deviceManager->Initialize(pCreateInfo->surfaceExtension);

	return deviceManager;
}

DeviceManager::~DeviceManager()
{
	if (logicalDevice)
	{
		vkDestroyDevice(logicalDevice, nullptr);
		logicalDevice = nullptr;
	}
	if (instance)
	{
		if (enableValidation && debugReportCallback) {
			auto destroyDebugReportCallbackEX = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
			if (destroyDebugReportCallbackEX)
			{
				destroyDebugReportCallbackEX(instance, debugReportCallback, nullptr);
				debugReportCallback = nullptr;
			}
		}

		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
	}
}

void DeviceManager::Initialize(const char * surfaceExtension)
{
	CreateInstance(surfaceExtension);
	CreateDebugReportCallback();
	GetPhysicalDevice();
	CreateLogicalDevice(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	pLogger->Info("DeviceManager initialized");
}

void DeviceManager::CreateInstance(const char * surfaceExtension)
{
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
	if (surfaceExtension)
	{
		enabledExtensions.push_back(surfaceExtension);
	}

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

	pLogger->Info("DeviceManager instance created");
}

void DeviceManager::CreateDebugReportCallback()
{
	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT; // VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT
	dbgCreateInfo.pfnCallback = [](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData) -> VkBool32
	{
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

		globalLogger->Error(message);

		fflush(stdout);

		// The return value of this callback controls wether the Vulkan call that caused
		// the validation message will be aborted or not
		// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
		// (and return a VkResult) to abort
		// If you instead want to have calls abort, pass in VK_TRUE and the function will 
		// return VK_ERROR_VALIDATION_FAILED_EXT 
		return VK_FALSE;
	};

	auto createDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (createDebugReportCallbackEXT(instance, &dbgCreateInfo, nullptr, &debugReportCallback) != VK_SUCCESS)
		throw "Failed to create debug report callback";

	pLogger->Info("DeviceManager DebugReportCallback created");
}

void DeviceManager::GetPhysicalDevice()
{
	unsigned int count = 0;
	if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS)
		throw "Failed to get physical devices count";

	pLogger->Info("DeviceManager found " + std::to_string(count) + " physical device(s)");

	std::vector<VkPhysicalDevice> physicalDevices(count);
	if (vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data()) != VK_SUCCESS)
		throw "Failed to get list of physical devices";

	// selects the first one, should check the device capabilities and select the most optimal one
	physicalDevice = physicalDevices[0];

	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	pLogger->Info("DeviceManager physical device properties loaded");

	std::string deviceName(physicalDeviceProperties.deviceName);
	pLogger->Info("DeviceManager selected physical device: " + deviceName);

	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	pLogger->Info("DeviceManager physical device features loaded");

	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	pLogger->Info("DeviceManager physical device memory properties loaded");

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

	pLogger->Info("DeviceManager physical device queuefamilyproperties loaded");
}

void DeviceManager::CreateLogicalDevice(VkQueueFlags requestedQueueTypes)
{
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

	pLogger->Info("DeviceManager logical device created");
}

unsigned int DeviceManager::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
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

unsigned int DeviceManager::GetMemoryType(unsigned int typeBits, VkMemoryPropertyFlags properties, VkBool32 * memTypeFound)
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
