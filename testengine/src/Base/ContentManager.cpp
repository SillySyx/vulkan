#include "ContentManager.h"

VkShaderModule ContentManager::LoadShaderSPIRV(VkDevice device, std::string filename)
{
	logger.Info("ContentManager::LoadShaderSPIRV " + filename);

	std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (is.is_open())
	{
		size_t size = is.tellg();
		is.seekg(0, std::ios::beg);
		char* shaderCode = new char[size];
		is.read(shaderCode, size);
		is.close();

		assert(size > 0);

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = NULL;
		moduleCreateInfo.flags = 0;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (uint32_t*)shaderCode;

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule) != VK_SUCCESS)
			throw "Failed to create shader module";

		logger.Info("ContentManager::LoadShaderSPIRV " + logger.GetAddr(shaderModule));

		delete[] shaderCode;

		return shaderModule;
	}
	else
	{
		throw "Could not open shader file: " + filename;
	}

	//
	//	size_t shaderSize;
	//	char* shaderCode;
	//
	//#if defined(__ANDROID__)
	//	// Load shader from compressed asset
	//	
	//#else
	//	std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);
	//
	//	if (is.is_open())
	//	{
	//		shaderSize = is.tellg();
	//		is.seekg(0, std::ios::beg);
	//		// Copy file contents into a buffer
	//		shaderCode = new char[shaderSize];
	//		is.read(shaderCode, shaderSize);
	//		is.close();
	//		assert(shaderSize > 0);
	//	}
	//#endif
	//	if (shaderCode)
	//	{
	//		// Create a new shader module that will be used for pipeline creation
	//		VkShaderModuleCreateInfo moduleCreateInfo{};
	//		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	//		moduleCreateInfo.codeSize = shaderSize;
	//		moduleCreateInfo.pCode = (uint32_t*)shaderCode;
	//
	//		VkShaderModule shaderModule;
	//		if (vk.CreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule) != VK_SUCCESS)
	//			throw "Failed to create shader module";
	//
	//		logger.Info("ContentManager::LoadShaderSPIRV " + logger.GetAddr(&shaderModule));
	//
	//		delete[] shaderCode;
	//
	//		return shaderModule;
	//	}
	//	else
	//	{
	//		throw "Could not open shader file: " + filename;
	//	}
	//
}
