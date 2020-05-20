#pragma once

#include <fstream>

#include "../Logging/LoggerConsole.h"
#include "../Vulkan/VulkanWrapper.h"

class ContentManager
{
public:

	VkShaderModule LoadShaderSPIRV(VkDevice device, std::string filename);

protected:

	LoggerConsole logger;
};