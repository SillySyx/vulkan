#pragma once
#include <vector>
#include <string>

#include "MeshManager.h"

struct Camera
{
	glm::vec3 position;
	glm::vec3 rotation;
};

struct SceneCreateInfo
{
	uint32_t activeCameraIndex;
	std::vector<Camera> cameras;

	std::vector<std::string> meshesToLoad;
};

struct Scene
{
	uint32_t activeCameraIndex;
	std::vector<Camera> cameras;
	
	std::vector<MeshInstance*> meshes;
};

class SceneManager
{

public:

	uint32_t activeSceneIndex;
	std::vector<Scene> scenes;

	void Initialize(MeshManager* meshManager);

	void CreateScene(SceneCreateInfo* createInfo);

protected:

	MeshManager* meshManager;

};