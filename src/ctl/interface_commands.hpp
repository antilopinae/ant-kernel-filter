#pragma once

#include <cstdint>
#include <string>
#include <variant>

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct Command1 {
    const std::uint64_t width, height;
    const std::string &output_filename;
};

struct Command2 {
    const std::string &first_filename;
};

using Command = std::variant<Command1, Command2>;