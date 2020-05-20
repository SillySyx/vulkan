#pragma once

#include "VulkanApplication.h"

class WindowApplication : VulkanApplication {
public:
	void Run();

protected:
	
	VulkanWindow* Window;

	void Initialize();

};