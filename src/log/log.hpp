#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <fmt/format.h>
#include <syslog.h>

namespace ant::log {
    enum class LogLevel : uint8_t {
        Debug = 0,
        Error = 1,
    };

    class Logger {
    public:
        // journalctl -t ant-log
        static auto Init() -> void;

        static auto DeInit() -> void;
    };

    template<typename T>
    static auto Debug(const T &message) -> void;

    template<typename T>
    static auto Error(const T &message) -> void;

    template<typename T>
    auto Log(LogLevel level, const T &message) -> void {
        std::cout << fmt::format("{}", message) << std::endl << std::flush;
    }

    template<typename T>
    static auto Debug(const T &message) -> void {
        Log(LogLevel::Debug, message);

        // Initialize syslog
        Logger::Init();
        syslog(LOG_DEBUG, message.c_str());
    }

    template<typename T>
    static auto Error(const T &message) -> void {
        Log(LogLevel::Error, message);

        // Initialize syslog
        Logger::Init();
        syslog(LOG_ERR, message.c_str());
    }
}
