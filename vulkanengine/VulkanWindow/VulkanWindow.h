#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "../VDeleter.h"

class VulkanWindow {
public:
	void Initialize(int width, int height, char* title);

	void Start();

	const char** GetRequiredInstanceExtensions(uint32_t *count);

	void CreateSurface(VkInstance instance, VkSurfaceKHR *surface);

protected:
	GLFWwindow* Window;

};
