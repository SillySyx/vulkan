#pragma once

#include <vector>

enum class Buttons
{
	Escape,
};

class ButtonState
{
public:
	std::vector<Buttons> pressedButtons;

	bool IsButtonPressed(Buttons button);
};

class BaseInputManager
{
public:
	virtual ButtonState GetState() = 0;
};