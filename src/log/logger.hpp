#pragma once

#include <cstdint>

namespace ant::log {
    enum class LogLevel : uint8_t {
        Debug = 0,
        Error = 1,
    };

    template<typename T>
    static auto Debug(const T& message) -> void;

    template<typename T>
    static auto Error(const T& message) -> void;

    class Logger {
    public:
        static auto Init() -> void;

        static auto DeInit() -> void;
    };
}
