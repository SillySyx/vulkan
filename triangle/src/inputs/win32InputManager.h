#pragma once

#include "baseInputManager.h"

#include <Windows.h>
#include <map>

class Win32InputManager : public BaseInputManager
{
public:
	ButtonState GetState() override;

protected:
	bool IsButtonPressed(Buttons button);
};