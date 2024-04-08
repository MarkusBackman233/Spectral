#pragma once
#include "pch.h"
#include "chrono"
#include "ProfileObject.h"

class ProfilerManager
{
public:
	static ProfilerManager* GetInstance() {
		static ProfilerManager instance;
		return &instance;
	}

	ProfilerManager();
	std::unordered_map<std::string, long long>& GetTimers() { return m_timers; }
	ProfileObject GetProfileObject(const char* functionName) { return ProfileObject(functionName); }

private:
	std::unordered_map<std::string, long long> m_timers;
};