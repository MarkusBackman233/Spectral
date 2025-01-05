#include "InputManager.h"
#include "StringUtils.h"


InputManager::InputManager()
{
	CreateStringToKeyMap();
}

void InputManager::Update()
{
	m_mouseMovementDeltaLastFrame = m_mouseMovementDelta;
	m_mouseMovementDelta = Math::Vector2(0.0f, 0.0f);
	for (auto& [key, state] : m_updatedKeyStates)
	{
		switch (state)
		{
		case KeyState::Released:
			state = KeyState::Inactive;
			break;
		case KeyState::Pressed:
			state = KeyState::Held;
			break;
		}
	}

	for (const auto& [key, incomingState] : m_bufferedKeyStates)
	{
		auto keyIterator = m_updatedKeyStates.find(key);
		if (keyIterator == m_updatedKeyStates.end())
		{
			auto [iterator, inserted] = m_updatedKeyStates.emplace(key, KeyState::Inactive);
			keyIterator = iterator;
		}

		KeyState& lastState = keyIterator->second;

		if (lastState == KeyState::Pressed && incomingState == KeyState::Pressed || 
			lastState == KeyState::Held && incomingState == KeyState::Pressed)
		{
			lastState = KeyState::Held;
		}
		else if (lastState == KeyState::Inactive && incomingState == KeyState::Pressed)
		{
			lastState = KeyState::Pressed;
		}
		else
		{
			lastState = incomingState;
		}
	}
	m_bufferedKeyStates.clear();
}


void InputManager::RegisterInput(InputId keyCode, KeyState keyState)
{
	m_bufferedKeyStates[keyCode] = keyState;
}

void InputManager::RegisterMousePosition(const Math::Vector2& mousePosition)
{
	m_mouseMovementDelta = m_mouseMovementDelta + mousePosition;
}

void InputManager::ClearInputs()
{
	for (auto& [key, state] : m_updatedKeyStates)
	{
		state = KeyState::Inactive;
	}
}

bool InputManager::GetInternalKeyPressed(InputId key)
{
	return GetKeyState(key) == KeyState::Pressed;
}

bool InputManager::GetInternalKeyReleased(InputId key)
{
	return GetKeyState(key) == KeyState::Released;
}

bool InputManager::GetInternalKeyHeld(InputId key)
{
	KeyState keyState = GetKeyState(key);
	return keyState == KeyState::Held || keyState == KeyState::Pressed;
}

InputId InputManager::ProccesKeyString(const std::string& key)
{
	if (key.size() == 1)
	{
		return static_cast<InputId>(std::toupper(key[0]));
	}

	auto keyIt = m_stringToKeyChar.find(StringUtils::StringToLower(key));
	AssertAndReturn(keyIt != m_stringToKeyChar.end(), "Key, (" + key + ") does not have a correct input to char conversion", return InputId(0));
	return keyIt->second;
}

void InputManager::CreateStringToKeyMap()
{
	m_stringToKeyChar = { 
		{"space", InputId::Space },
		{"escape", InputId::Escape },
		{"shift", InputId::Shift },
		{"control", InputId::Control },
		{"up", InputId::Up },
		{"down", InputId::Down },
		{"left", InputId::Left },
		{"right", InputId::Right },
		{"mouse1", InputId::Mouse1 },
		{"mouse2", InputId::Mouse2 },
		{"mouse3", InputId::Mouse3 },
	};
}

Math::Vector2 InputManager::GetMouseMovement() const
{
	return m_mouseMovementDeltaLastFrame;
}

bool InputManager::GetKeyPressed(const std::string& key)
{
	return GetInternalKeyPressed(ProccesKeyString(key));
}

bool InputManager::GetKeyReleased(const std::string& key)
{
	return GetInternalKeyReleased(ProccesKeyString(key));
}

bool InputManager::GetKeyHeld(const std::string& key)
{
	return GetInternalKeyHeld(ProccesKeyString(key));
}


InputManager::KeyState InputManager::GetKeyState(InputId key)
{
	if (m_updatedKeyStates.find(key) != m_updatedKeyStates.end())
	{
		return m_updatedKeyStates.find(key)->second;
	}

	return KeyState::Inactive;
}
