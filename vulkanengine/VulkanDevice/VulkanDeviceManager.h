#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include "../VulkanWindow/VulkanWindow.h"
#include "../VDeleter.h"

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanDeviceManager
{
public:
	std::vector<const char*> GetRequiredExtensions(VulkanWindow *window);

	void PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);

	void CreateLogicalDevice(VkSurfaceKHR surface);

	void CreateSwapChain(VkSurfaceKHR surface, uint32_t width, uint32_t height);

	void CreateImageViews();

protected:
	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VDeleter<VkDevice> device{ vkDestroyDevice };
	VDeleter<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
	std::vector<VDeleter<VkImageView>> swapChainImageViews;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);
};
