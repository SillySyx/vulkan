#include "WindowManager.h"

void WindowManager::Initialize(char* title, int width, int height, GLFWwindowsizefun resizeCallback)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, resizeCallback);
}

void WindowManager::SetWindowMode(WindowModes mode, int width, int height)
{
	if (window == nullptr)
		throw std::runtime_error("window null reference");

	if (mode == WindowModes::Window)
	{
		glfwSetWindowMonitor(window, NULL, 100, 100, width, height, GLFW_DONT_CARE);
	}
	else
	{
		auto monitor = glfwGetPrimaryMonitor();
		if (mode == WindowModes::Borderless)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

			glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			glfwSetWindowMonitor(window, monitor, 0, 0, width, height, mode->refreshRate);
		}
	}
}

void WindowManager::CreateSurface(VkInstance instance)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
}

WindowManager::~WindowManager()
{
}
