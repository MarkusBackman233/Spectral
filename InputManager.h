#pragma once
#include "pch.h"
#include "Vector2.h"
#include "InputIds.h"

class InputManager
{
public:
	enum KeyState : uint8_t
	{
		Inactive,
		Pressed,
		Released,
		Held,
	};

	static InputManager* GetInstance() {
		static InputManager instance;
		return &instance;
	}

	InputManager();
	void Update();
	void RegisterInput(InputId key, KeyState keyState);
	void RegisterMousePosition(const Math::Vector2& mousePosition);
	void ClearInputs();

	Math::Vector2 GetMouseMovement() const;

	bool GetKeyPressed(const std::string& key);
	bool GetKeyReleased(const std::string& key);
	bool GetKeyHeld(const std::string& key);

	bool GetInternalKeyPressed(InputId key);
	bool GetInternalKeyReleased(InputId key);
	bool GetInternalKeyHeld(InputId key);

private:


	InputId ProccesKeyString(const std::string& key);

	void CreateStringToKeyMap();

	KeyState GetKeyState(InputId key);

	Math::Vector2 m_mouseMovementDelta;
	Math::Vector2 m_mouseMovementDeltaLastFrame;
	
	std::unordered_map<std::string, InputId> m_stringToKeyChar;

	// These are raw input from the winapi proc
	std::unordered_map<InputId, KeyState> m_bufferedKeyStates;

	// These have been processed in the update loop and are the ones that actually are read from GetKey functions
	std::unordered_map<InputId, KeyState> m_updatedKeyStates;
};

