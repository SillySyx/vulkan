#pragma once

#include "..\windows\baseWindow.h"
#include "..\inputs\baseInputManager.h"

struct SceneCreateInfo
{
public:
	BaseWindow * pWindow;
	BaseInputManager * pInputManager;
};

class Scene
{
public:
	BaseWindow * pWindow;
	BaseInputManager * pInputManager;

	void Update();

protected:
	ButtonState prevButtonState;
};