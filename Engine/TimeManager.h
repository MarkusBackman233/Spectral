#pragma once
#include "chrono"
class TimeManager
{
public:
	TimeManager();
	void Update();

	static float GetDeltaTime();
	static float GetLifeTime();
private:

	std::chrono::steady_clock::time_point m_previousTime;
	static float DeltaTime;
	static float LifeTime;
};

