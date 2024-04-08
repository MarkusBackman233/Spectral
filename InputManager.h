#pragma once
#include "pch.h"
class InputManager
{
public:
	static InputManager* GetInstance() {
		static InputManager instance;
		return &instance;
	}

	InputManager();

	void RegisterInput();
};

