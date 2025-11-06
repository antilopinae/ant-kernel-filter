#include <logger.hpp>

#include <iostream>
#include <fmt/format.h>
#include <syslog.h>

namespace ant::log {
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

    template void Debug<std::string>(const std::string &message);

    template void Error<std::string>(const std::string &message);

    auto Logger::Init() -> void {
        static auto p = [] -> bool {
            openlog("ant-log", LOG_PID | LOG_CONS, LOG_USER);
            return true;
        }();
    }

    auto Logger::DeInit() -> void {
        closelog();
    }
}
