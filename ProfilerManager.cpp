#include "ProfilerManager.h"

ProfilerManager::ProfilerManager()
	: m_collectionPaused(false)
{
	
}

void ProfilerManager::SetTime(const std::string& timerName, long long microSeconds)
{
	if (m_collectionPaused)
	{
		return;
	}

	auto it = m_timers.find(timerName);

	if (it != m_timers.end())
	{
		it->second.push_back(microSeconds);

		if (it->second.size() > 100)
		{
			it->second.pop_front();
		}

	}
	else
	{ 
		m_timers.emplace(timerName, std::deque<long long>{ microSeconds });
	}
}

void ProfilerManager::SetCollectionPaused(bool pause)
{
	m_collectionPaused = pause;
}
