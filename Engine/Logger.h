#pragma once
#include "pch.h"
#include <mutex>

class Logger
{
public:
	static void Info(const std::string& message);
	static void Error(const std::string& message);

	std::deque<std::string>& GetLog();
private:
	static Logger* GetInstance() {
		static Logger instance;
		return &instance;
	}
	std::deque<std::string> m_logBuffer;
	static std::mutex m_mutex;
};
