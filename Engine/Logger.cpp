#include "Logger.h"
std::mutex Logger::m_mutex;
std::deque<std::string> Logger::m_logBuffer;

std::deque<std::string>& Logger::GetLog()
{
    return m_logBuffer;
}