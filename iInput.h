#pragma once
#include "InputIds.h"
#include "Vector2.h"

namespace Input
{
	bool GetKeyPressed(InputId inputId);
	bool GetKeyHeld(InputId inputId);
	bool GetKeyReleased(InputId inputId);
	Math::Vector2 GetMouseMovement();
};

