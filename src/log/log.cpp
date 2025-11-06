#include <log.hpp>

#include <iostream>
#include <fmt/format.h>
#include <syslog.h>

namespace ant::log {
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
