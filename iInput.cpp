#include "iInput.h"
#include "InputManager.h"
namespace Input
{
	bool GetKeyPressed(InputId inputId)
	{
		return InputManager::GetInstance()->GetInternalKeyPressed(inputId);
	}
	bool GetKeyHeld(InputId inputId)
	{
		return InputManager::GetInstance()->GetInternalKeyHeld(inputId);
	}
	bool GetKeyReleased(InputId inputId)
	{
		return InputManager::GetInstance()->GetInternalKeyReleased(inputId);
	}
	Math::Vector2 GetMouseMovement()
	{
		return InputManager::GetInstance()->GetMouseMovement();
	}
};
