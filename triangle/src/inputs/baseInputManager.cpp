#include "baseInputManager.h"

bool ButtonState::IsButtonPressed(Buttons button)
{
	for (auto btn : pressedButtons)
	{
		if (btn == button)
			return true;
	}
	return false;
}