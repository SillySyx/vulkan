#include "SceneManager.h"

void SceneManager::Initiate(void * renderer)
{
	this->renderer = (RendererForward*)renderer;

	scene.Initiate(renderer, &contentManager);
}

void SceneManager::Update(double deltaTimer)
{
	scene.Update(deltaTimer);
}

void SceneManager::Render()
{
	renderer->Render([=]
	{
		scene.Render();
	});
}

void SceneManager::ResizeResources(uint32_t width, uint32_t height)
{
	renderer->ResizeResources(width, height);
	scene.ResizeResources(width, height);
}
