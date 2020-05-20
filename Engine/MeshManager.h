#pragma once
#include <vector>
#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkan\vulkan.h"

#include "DeviceManager.h"

struct Vertex
{
	float position[3];
	float color[3];
};

struct MeshInstance
{
	std::string name;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

	uint32_t indexCount;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
};

class MeshManager
{

public:

	void Initialize(DeviceManager* deviceManager, VkQueue* graphicsQueue);

	MeshInstance* getInstance(std::string name);

	~MeshManager();

protected:

	DeviceManager* deviceManager;
	VkQueue* graphicsQueue;

	std::vector<MeshInstance> instances;

	MeshInstance* loadMesh(std::string name);
	
	void buildBuffers(MeshInstance* instance, std::vector<Vertex> verticies, std::vector<uint32_t> indices);
};