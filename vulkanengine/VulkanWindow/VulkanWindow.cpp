#include "VulkanWindow.h"

void VulkanWindow::Initialize(int width, int height, char* title)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void VulkanWindow::Start()
{
	while (!glfwWindowShouldClose(Window))
		glfwPollEvents();

	glfwDestroyWindow(Window);

	glfwTerminate();
}

const char** VulkanWindow::GetRequiredInstanceExtensions(uint32_t *count)
{
	return glfwGetRequiredInstanceExtensions(count);
}

void VulkanWindow::CreateSurface(VkInstance instance, VkSurfaceKHR *surface)
{
	if (glfwCreateWindowSurface(instance, Window, nullptr, surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}