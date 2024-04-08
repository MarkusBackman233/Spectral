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
		ProfilerManager::GetInstance()->GetTimers()[timerName] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - StartTime).count();
	}
	else
	{
		std::cout << "Error: timer, " << timerName << " was not started!" << std::endl;
	}
}

void ProfileObject::StartTimer()
{
	StartTime = std::chrono::high_resolution_clock::now();
	m_timerStarted = true;
}
