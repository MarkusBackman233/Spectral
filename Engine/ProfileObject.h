#pragma once
#include "pch.h"
#include "chrono"

struct ProfileObject
{
	ProfileObject(const char* functionName);
	~ProfileObject();
	void StartTimer();

private:
	std::string timerName;
	std::chrono::time_point<std::chrono::steady_clock> StartTime;
	bool m_timerStarted;

};