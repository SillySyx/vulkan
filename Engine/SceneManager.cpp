#include "SceneManager.h"

void SceneManager::Initialize(MeshManager* meshManager)
{
	this->meshManager = meshManager;
}

void SceneManager::CreateScene(SceneCreateInfo* createInfo)
{
	Scene s;

	for (auto meshName : createInfo->meshesToLoad)
	{
		auto meshInstance = meshManager->getInstance(meshName);
		s.meshes.push_back(meshInstance);
	}

	s.activeCameraIndex = 0;
	for (auto camera : createInfo->cameras)
		s.cameras.push_back(camera);

	scenes.push_back(s);
}
