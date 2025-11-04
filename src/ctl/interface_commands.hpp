#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <fmt/format.h>

namespace ant::ctl {
    template<class... Ts>
    struct Overloaded : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;

    struct Command {
    };

    struct LoadEbpfModule : Command {
        static constexpr auto kDescription = "Loading eBPF module";
    };

    struct UnloadEbpfModule : Command {
        static constexpr auto kDescription = "Unloading eBPF module";
    };

    struct StatusEbpfModule : Command {
        static constexpr auto kDescription = "Status eBPF module";
    };

    struct LoadKernelModule : Command {
        static constexpr auto kDescription = "Loading Kernel module";
    };

    struct UnloadKernelModule : Command {
        static constexpr auto kDescription = "Unloading Kernel module";
    };

    struct StatusKernelModule : Command {
        static constexpr auto kDescription = "Status Kernel module";
    };

    struct FilterLoad : Command {
        static constexpr auto kDescription = "Loading Filter";
        const std::string &filter_filename;
    };

    struct FilterUnload : Command {
        static constexpr auto kDescription = "Unloading Filter";
        const std::string &filter_name;
    };

    struct StatusFilterModule : Command {
        static constexpr auto kDescription = "Status Filter module";
    };

    struct StartPrometheus : Command {
        static constexpr auto kDescription = "Starting Prometheus";
    };

    struct StopPrometheus : Command {
        static constexpr auto kDescription = "Stopping Prometheus";
    };

    using InterfaceCommands = std::variant<
        LoadEbpfModule,
        UnloadEbpfModule,
        StatusEbpfModule,
        LoadKernelModule,
        UnloadKernelModule,
        StatusKernelModule,
        FilterLoad,
        FilterUnload,
        StatusFilterModule,
        StartPrometheus,
        StopPrometheus
    >;
}

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
