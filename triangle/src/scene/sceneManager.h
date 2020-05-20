#pragma once

#include "scene.h"

#include <vector>

class SceneManager
{
public:
	void Update();

	void AddScene(SceneCreateInfo * pCreateInfo);

protected:
	std::vector<Scene> scenes;
};