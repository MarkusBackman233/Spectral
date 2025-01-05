#include "TimeManager.h"
#include "chrono"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::steady_clock::time_point Time;

float TimeManager::DeltaTime = 0.0f;
float TimeManager::LifeTime = 0.0f;

TimeManager::TimeManager() 
    : m_previousTime(Clock::now())
{
}

void TimeManager::Update()
{
    Time currentTime = Clock::now();
    DeltaTime = std::chrono::duration<float>(currentTime - m_previousTime).count();
    LifeTime += DeltaTime;
    m_previousTime = currentTime;
}

float TimeManager::GetDeltaTime()
{
    return DeltaTime;
}

float TimeManager::GetLifeTime()
{
    return LifeTime;
}
