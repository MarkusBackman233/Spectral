#include "Logger.h"
#include <mutex>

void Logger::Info(const std::string& message)
{
    std::cout << "[Info] " << message << std::endl;
#ifdef EDITOR
    //static std::mutex mutex;
    //mutex.lock();
    //std::cout << message << "\n";
    //GetInstance()->GetLog().push_back(message);
    //while (GetInstance()->GetLog().size() > 250)
    //    GetInstance()->GetLog().pop_front();
    //mutex.unlock();
#endif // EDITOR
}

void Logger::Error(const std::string& message)
{
    std::cout << "\033[31m[Error] "<< message << "\033[0m" << std::endl;
}

std::deque<std::string>& Logger::GetLog()
{
    return m_logBuffer;
}
