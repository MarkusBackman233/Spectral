#pragma once
enum class SimulationState
{
	Starting,
	Resetting,
	Running,
	Stopped,
};

class SimulationStateManager
{
public:

	SimulationStateManager();

	void Update(float deltaTime);

	SimulationState GetSimulationState();

private:
	bool m_wasRunning;
};
