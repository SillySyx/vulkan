#include "win32InputManager.h"

std::map<Buttons, int> win32ButtonMap = 
{
	{ Buttons::Escape, VK_ESCAPE },
};

ButtonState Win32InputManager::GetState()
{
	ButtonState state = {};

	if (IsButtonPressed(Buttons::Escape))
		state.pressedButtons.push_back(Buttons::Escape);

	return state;
}

bool Win32InputManager::IsButtonPressed(Buttons button)
{
	return GetAsyncKeyState(win32ButtonMap[button]);
}
