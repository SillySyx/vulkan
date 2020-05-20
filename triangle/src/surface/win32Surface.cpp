#include "win32Surface.h"

Win32Surface * Win32Surface::Create(Win32SurfaceCreateInfo * pCreateInfo)
{
	auto surface = new Win32Surface;
	surface->pDevice = pCreateInfo->pDevice;
	surface->pLogger = pCreateInfo->pLogger;

	surface->Initialize(pCreateInfo->hInstance, pCreateInfo->hwnd);
	
	return surface;
}

void Win32Surface::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = hwnd;

	if (vkCreateWin32SurfaceKHR(pDevice->instance, &surfaceCreateInfo, nullptr, &surface))
		throw "failed to create win32 surface";

	pLogger->Info("Win32Surface surface created");

	// Get list of supported surface formats
	unsigned int formatCount;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice->physicalDevice, surface, &formatCount, NULL) != VK_SUCCESS)
		throw "Failed to get physical device surface formats count";

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice->physicalDevice, surface, &formatCount, surfaceFormats.data()) != VK_SUCCESS)
		throw "Failed to get physical device surface formats data";

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		// Always select the first available color format
		// If you need a specific format (e.g. SRGB) you'd need to
		// iterate over the list of available surface format and
		// check for it's presence
		colorFormat = surfaceFormats[0].format;
	}
	colorSpace = surfaceFormats[0].colorSpace;

	pLogger->Info("Win32Surface color format loaded");
}