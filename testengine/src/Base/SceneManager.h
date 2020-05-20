#pragma once

#include "../Renderers/RendererForward.h"
#include "../Scenes/TriangleScene.h"
#include "ContentManager.h"

class SceneManager
{

public:

	void Initiate(void * renderer);

	void Update(double deltaTimer);
	void Render();

	void ResizeResources(uint32_t width, uint32_t height);

protected:

	ContentManager contentManager;
	TriangleScene scene;
	
	RendererForward * renderer = VK_NULL_HANDLE;

};