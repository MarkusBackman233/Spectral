#include "Logger.h"

std::mutex Logger::m_mutex;


void Logger::Info(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[Info] " << message << std::endl;
}

void Logger::Error(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "\033[31m[Error] "<< message << "\033[0m" << std::endl;
}

std::deque<std::string>& Logger::GetLog()
{
    return m_logBuffer;
}
