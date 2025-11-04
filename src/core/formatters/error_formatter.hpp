#pragma once

#include <error.hpp>

template<>
struct fmt::formatter<ant::core::Error> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) const {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const ant::core::Error &var, FormatContext &ctx) const {
            return fmt::format_to(ctx.out(), "{}", v.What());
    }
};
