#pragma once
enum class GameState : unsigned char
{
	Starting,
	Resetting,
	Running,
	Stopped,
};

class GameStateManager
{
public:
	GameStateManager();

	void Update(float deltaTime);

	GameState GetGameState() const;

private:

	void UpdateGameState();

	GameState m_gameState;
};
