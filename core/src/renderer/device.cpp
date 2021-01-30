#include "renderer/device.h"

std::vector<const char*> load_extensions(bool enableValidation)
{
	std::vector<const char*> extensions;

	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

	#ifdef VK_USE_PLATFORM_XLIB_KHR
	extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
	#endif

	#ifdef VK_USE_PLATFORM_XCB_KHR
	extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	#endif

	#ifdef VK_USE_PLATFORM_WIN32_KHR
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#endif

	if (enableValidation)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

std::vector<const char*> load_layers(bool enableValidation)
{
	std::vector<const char*> layers;

	if (enableValidation)
	{
		layers.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	return layers;
}

bool create_instance(CreateDeviceInfo* create_device_info, Device* device)
{
    VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = create_device_info->application_name;
	appInfo.pEngineName = create_device_info->engine_name;
	appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;

	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	auto extensions = load_extensions(create_device_info->enableValidation);
	instanceCreateInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	auto layers = load_layers(create_device_info->enableValidation);
	instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
	instanceCreateInfo.ppEnabledLayerNames = layers.data();

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &device->instance) != VK_SUCCESS)
		return false;
}

bool create_debug_report_callback(CreateDeviceInfo* create_device_info, Device* device)
{
	if (!create_device_info->enableValidation)
		return true;

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
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

		std::cout << message << std::endl;

		fflush(stdout);

		// The return value of this callback controls wether the Vulkan call that caused
		// the validation message will be aborted or not
		// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
		// (and return a VkResult) to abort
		// If you instead want to have calls abort, pass in VK_TRUE and the function will 
		// return VK_ERROR_VALIDATION_FAILED_EXT 
		return VK_FALSE;
	};

	if (vkCreateDebugReportCallbackEXT(device->instance, &dbgCreateInfo, nullptr, &device->debugReportCallback) != VK_SUCCESS)
		return false;

	return true;
}

bool create_device(CreateDeviceInfo* create_device_info, Device* device)
{
	if (!create_instance(create_device_info, device))
		return false;

	if (!create_debug_report_callback(create_device_info, device))
		return false;

	// PhysicalDevice
	// LogicalDevice

    return true;
}

void destroy_device(Device* device)
{
	if (device->debugReportCallback)
	{
		vkDestroyDebugReportCallbackEXT(device->instance, device->debugReportCallback, nullptr);
		device->debugReportCallback = nullptr;
	}

	vkDestroyInstance(device->instance, nullptr);
}