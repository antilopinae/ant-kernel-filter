#pragma once

#include <fmt/format.h>
#include <ctl/commands.hpp>

template<typename T>
    requires std::is_base_of_v<ant::ctl::Command, T>
struct fmt::formatter<T> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) const {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const T &val, FormatContext &ctx) const {
        constexpr std::string_view desc{T::kDescription};
        return fmt::format_to(ctx.out(), "{}", desc);
    }
};
