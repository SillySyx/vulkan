#include "sceneManager.h"

void SceneManager::Update()
{
	for (auto scene : scenes)
	{
		scene.Update();
	}
}

void SceneManager::AddScene(SceneCreateInfo * pCreateInfo)
{
	Scene scene = {};
	scene.pInputManager = pCreateInfo->pInputManager;
	scene.pWindow = pCreateInfo->pWindow;

	scenes.push_back(scene);
}