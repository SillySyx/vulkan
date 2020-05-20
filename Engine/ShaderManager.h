#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include <vulkan\vulkan.h>

class ShaderManager
{
public:

	VkShaderModule CreateShaderModule(VkDevice device, char* filePath);

};