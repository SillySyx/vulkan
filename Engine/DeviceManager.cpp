#include "DeviceManager.h"

void DeviceManager::Initialize(char* applicationName, char* engineName, bool enableValidation, VkQueueFlags requestedQueueTypes)
{
	validationEnabled = enableValidation;

	if (CreateInstance(applicationName, engineName) != VK_SUCCESS)
		throw std::runtime_error("failed to create instance");

	if (validationEnabled)
		if (CreateDebugCallback(VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_NULL_HANDLE) != VK_SUCCESS)
			throw std::runtime_error("failed to create debug callback");

	if (GetPhysicalDevice() != VK_SUCCESS)
		throw std::runtime_error("failed to get physical device");

	if (CreateLogicalDevice(requestedQueueTypes) != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device");

	commandPool = CreateCommandPool(queueFamilyIndices.graphics);
}

VkCommandPool DeviceManager::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.flags = createFlags;
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
	
	VkCommandPool cmdPool;
	if (vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create command buffer");
	
	return cmdPool;
}

DeviceManager::~DeviceManager()
{
	if (validationEnabled && debugReportCallback != VK_NULL_HANDLE) {
		auto destroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
		destroyDebugReportCallback(instance, debugReportCallback, nullptr);
	}

	if (commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		commandPool = VK_NULL_HANDLE;
	}

	if (logicalDevice) {
		vkDestroyDevice(logicalDevice, nullptr);
		logicalDevice = NULL;
	}

	if (instance) {
		vkDestroyInstance(instance, nullptr);
		instance = NULL;
	}
}

VkResult DeviceManager::CreateInstance(char * applicationName, char * engineName)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = applicationName;
	appInfo.pEngineName = engineName;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> enabledExtensions;

	unsigned int glfwExtensionCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
		enabledExtensions.push_back(glfwExtensions[i]);

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (enabledExtensions.size() > 0)
	{
		if (validationEnabled)
			enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}

	if (validationEnabled)
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

	return vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}

VkResult DeviceManager::CreateDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack)
{
	auto createDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pfnCallback = DebugMessageCallback;
	dbgCreateInfo.flags = flags;

	return createDebugReportCallback(instance, &dbgCreateInfo, nullptr, (callBack != VK_NULL_HANDLE) ? &callBack : &debugReportCallback);
}

VkResult DeviceManager::GetPhysicalDevice()
{
	uint32_t count = 0;
	auto error = vkEnumeratePhysicalDevices(instance, &count, nullptr);
	if (error) return error;

	std::vector<VkPhysicalDevice> physicalDevices(count);
	error = vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());
	if (error) return error;

	// selects the first one, should check the device capabilities and select the most optimal one
	physicalDevice = physicalDevices[0];

	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

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

	return VK_SUCCESS;
}

VkResult DeviceManager::CreateLogicalDevice(VkQueueFlags requestedQueueTypes)
{
	const float defaultQueuePriority(0.0f);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	}

	// Dedicated compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
	{
		queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
		if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else
	{
		// Else we use the same queue
		queueFamilyIndices.compute = queueFamilyIndices.graphics;
	}

	// Dedicated transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
	{
		queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
		if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else
	{
		// Else we use the same queue
		queueFamilyIndices.transfer = queueFamilyIndices.graphics;
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

	return vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
}

uint32_t DeviceManager::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
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

	throw std::runtime_error("Could not find a matching queue family index");
}

uint32_t DeviceManager::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	// Iterate over all memory types available for the device used in this example
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}

	throw std::runtime_error("Could not find a suitable memory type");
}

VkBool32 DeviceManager::DebugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData)
{
	// Select prefix depending on flags passed to the callback
	// Note that multiple flags may be set for a single validation message
	std::string prefix("");

	// Error that may result in undefined behaviour
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		prefix += "ERROR:";

	// Warnings may hint at unexpected / non-spec API usage
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		prefix += "WARNING:";

	// May indicate sub-optimal usage of the API
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		prefix += "PERFORMANCE:";

	// Informal messages that may become handy during debugging
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		prefix += "INFO:";

	// Diagnostic info from the Vulkan loader and layers
	// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		prefix += "DEBUG:";

	// Display message to default output (console if activated)
	std::cout << prefix << " [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg << "\n";

	fflush(stdout);

	// The return value of this callback controls wether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}