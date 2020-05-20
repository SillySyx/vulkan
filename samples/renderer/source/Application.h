#pragma once

#include "VulkanApplication.h"
#include "VulkanRenderer\VulkanForwardRenderer.h"

class Application : VulkanApplication {
public:
	void Run();

protected:
	void Initialize();
};