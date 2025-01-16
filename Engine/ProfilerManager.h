#pragma once
#include "pch.h"
#include "chrono"
#include "ProfileObject.h"

#define ProfileFunction \
auto timer = ProfilerManager::GetInstance()->GetProfileObject(std::string(std::string(typeid(*this).name())+"::" +std::string(__func__)).c_str()); \
timer.StartTimer(); 


class ProfilerManager
{
public:
	static ProfilerManager* GetInstance() {
		static ProfilerManager instance;
		return &instance;
	}

	ProfilerManager();
	const std::unordered_map<std::string, std::deque<long long>>& GetTimers() { return m_timers; }

	void SetTime(const std::string& timerName, long long microSeconds);

	ProfileObject GetProfileObject(const char* functionName) { return ProfileObject(functionName); }

	void SetCollectionPaused(bool pause);
	bool IsCollectionPaused() const { return m_collectionPaused; }
private:
	std::unordered_map<std::string, std::deque<long long>> m_timers;
	bool m_collectionPaused;
};