#pragma once
#include "pch.h"
#include <string>
#include <deque>
#include <mutex>
#include <format>
#include <print>

class Logger
{
public:
    template <typename... Args>
    static void Info(std::string_view fmt, Args&&... args)
    {
        std::lock_guard lock(m_mutex);
        std::string msg = std::vformat(fmt, std::make_format_args(args...));
        std::println("[Info] {}", msg);
        m_logBuffer.push_back("[Info] " + msg);
    }

    template <typename... Args>
    static void Error(std::string_view fmt, Args&&... args)
    {
        std::lock_guard lock(m_mutex);
        std::string msg = std::vformat(fmt, std::make_format_args(args...));
        std::println("\033[31m[Error] {}\033[0m", msg);
        m_logBuffer.push_back("[Error] " + msg);
    }

    static std::deque<std::string>& GetLog();

private:
    static std::mutex m_mutex;
    static std::deque<std::string> m_logBuffer;
};