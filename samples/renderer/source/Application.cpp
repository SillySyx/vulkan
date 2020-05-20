#include "Application.h"

void Application::Run()
{
	Initialize();

	Window->Start();
}

void Application::Initialize()
{
	Window = new VulkanWindow();
	Window->Initialize(800, 600, "Vulkan renderer application");
	
	CreateInstance();
	Window->CreateSurface(Instance, &Surface);

	DeviceManager = new VulkanDeviceManager();
	DeviceManager->PickPhysicalDevice(Instance, Surface);
	DeviceManager->CreateLogicalDevice(Surface);
	DeviceManager->CreateSwapChain(Surface, 800, 600);

	Renderer = new VulkanForwardRenderer();
}