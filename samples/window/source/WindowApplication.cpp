#include "WindowApplication.h"

void WindowApplication::Run()
{
	Initialize();

	Window->Start();
}

void WindowApplication::Initialize()
{
	Window = new VulkanWindow();
	Window->Initialize(800, 600, "Vulkan window application");

	//Engine = new VulkanEngine;
	//Engine->Initialize();
}