#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace ant::ctl {
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
