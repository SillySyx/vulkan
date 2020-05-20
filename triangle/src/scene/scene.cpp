#include "scene.h"

void Scene::Update()
{
	auto currentButtonState = pInputManager->GetState();
	if (!prevButtonState.IsButtonPressed(Buttons::Escape) && currentButtonState.IsButtonPressed(Buttons::Escape))
	{
		pWindow->shutdown = true;
	}
}