#include "Application.h"

void Application::Run()
{
	Initialize();

	Window->Start();
}

void Application::Initialize()
{
	Window = new VulkanWindow();
	Window->Initialize(800, 600, "Vulkan device application");

	CreateInstance();

	DeviceManager = new VulkanDeviceManager();
	DeviceManager->PickPhysicalDevice(Instance);
	DeviceManager->CreateLogicalDevice();

	//Engine = new VulkanEngine;
	//Engine->Initialize();
}