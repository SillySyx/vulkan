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

bool create_device(CreateDeviceInfo* create_device_info, Device* device)
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

	if (create_device_info->enableValidation)
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

	VkInstance instance;
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
		return false;

	device->instance = instance;

    return true;
}