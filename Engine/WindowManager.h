#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <memory>

enum WindowModes
{
	Window,
	Borderless,
	Fullscreen
};

class WindowManager
{
public:
	GLFWwindow* window;

	VkSurfaceKHR surface;

	void Initialize(char* title, int width, int height, GLFWwindowsizefun resizeCallback);

	void SetWindowMode(WindowModes mode, int width, int height);

	void CreateSurface(VkInstance instance);

	~WindowManager();
};