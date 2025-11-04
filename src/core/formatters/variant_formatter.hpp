#pragma once

#include <variant>
#include <fmt/format.h>

template<typename... Ts>
struct fmt::formatter<std::variant<Ts...> > {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) const {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const std::variant<Ts...> &var, FormatContext &ctx) const {
        return std::visit([&ctx](auto const &v) {
            return fmt::format_to(ctx.out(), "{}", v);
        }, var);
    }
};
