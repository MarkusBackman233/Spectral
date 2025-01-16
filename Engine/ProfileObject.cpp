#include "ProfileObject.h"
#include "ProfilerManager.h"

ProfileObject::ProfileObject(const char* functionName)
	: timerName(functionName)
	, m_timerStarted(false)
{
}

ProfileObject::~ProfileObject()
{
	if (m_timerStarted)
	{
		long long time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - StartTime).count();
		ProfilerManager::GetInstance()->SetTime(timerName, time);
	}
	else
	{
		Logger::Error("Timer, " + timerName + " was not started!");
	}
}

void ProfileObject::StartTimer()
{
	StartTime = std::chrono::high_resolution_clock::now();
	m_timerStarted = true;
}
