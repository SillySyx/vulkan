#include "BaseApplication.h"

BaseApplication* app;

void BaseApplication::Initialize(char* title, int width, int height)
{
	app = this;
	this->width = width;
	this->height = height;

	windowManager.Initialize(title, width, height, WindowResized);

	deviceManager.Initialize(title, "Hello Engine", enableValidation);
}

void BaseApplication::Run()
{
	if (windowManager.window == nullptr)
		throw std::runtime_error("window not initialized");

	Load();

	glfwSetInputMode(windowManager.window, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(windowManager.window, GLFW_STICKY_MOUSE_BUTTONS, 1);

	auto prevTime = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(windowManager.window) && !shouldShutDown) {
		glfwPollEvents();

		auto currentTime = std::chrono::high_resolution_clock::now();
		auto time = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime).count();
		
		auto fps = 1000000.f / time;
		
		// std::cout << "fps: " << 1000000.f / time << std::endl;
/*
		std::stringstream ss;
		ss << "fps: " << fps;
		glfwSetWindowTitle(windowManager.window, ss.str().c_str());
*/
		Update(time);

		DrawFrame();

		//_sleep(10);

		prevTime = currentTime;
	}

	glfwDestroyWindow(windowManager.window);

	vkDeviceWaitIdle(deviceManager.logicalDevice);
}

BaseApplication::~BaseApplication()
{
	vkDestroySurfaceKHR(deviceManager.instance, windowManager.surface, nullptr);
	windowManager.surface = NULL;
}

void BaseApplication::WindowResized(GLFWwindow * window, int width, int height)
{
	if (width <= 0 || height <= 0) return;

	app->width = width;
	app->height = height;
	app->WindowSizeChanged();
}
