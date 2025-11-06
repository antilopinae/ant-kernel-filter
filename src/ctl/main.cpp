#include <ctl/commands.hpp>
#include <ctl/formatters/commands_formatter.hpp>
#include <core/formatters/variant_formatter.hpp>
#include <core/overloaded.hpp>
#include <core/formatters/error_formatter.hpp>
#include <log/log.hpp>
#include <kernel/kernel-module.hpp>
#include <ant-kernel/headers.h>

#include <expected>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>
#include <fmt/format.h>

int main(int argc, char *argv[]) {
    argparse::ArgumentParser main_parser(ANT_PROGRAM_NAME, "0.1.0", argparse::default_arguments::help);
    main_parser.add_description("Kernel filter program");

    // Ebpf module
    argparse::ArgumentParser ebpf_module_command("ebpf");
    ebpf_module_command.add_description("Ebpf module management");

    argparse::ArgumentParser ebpf_load("load");
    ebpf_load.add_description("Load eBPF module");
    ebpf_module_command.add_subparser(ebpf_load);

    argparse::ArgumentParser ebpf_unload("unload");
    ebpf_unload.add_description("Unload eBPF module");
    ebpf_module_command.add_subparser(ebpf_unload);

    argparse::ArgumentParser ebpf_status("status");
    ebpf_status.add_description("Ebpf module status");
    ebpf_module_command.add_subparser(ebpf_status);

    // Kernel module
    argparse::ArgumentParser kernel_module_command("kernel");
    kernel_module_command.add_description("Kernel module management");

    argparse::ArgumentParser kernel_load("load");
    kernel_load.add_description("Load Kernel module");
    kernel_module_command.add_subparser(kernel_load);

    argparse::ArgumentParser kernel_unload("unload");
    kernel_unload.add_description("Unload Kernel module");
    kernel_module_command.add_subparser(kernel_unload);

    argparse::ArgumentParser kernel_status("status");
    kernel_status.add_description("Kernel module status");
    kernel_module_command.add_subparser(kernel_status);

    // Filters
    argparse::ArgumentParser filter_module_command("filter");
    filter_module_command.add_description("Kernel | eBPF module filter");

    std::string load_filter_filename;
    argparse::ArgumentParser filter_load("load");
    filter_load.add_description("Load filter");
    filter_load.add_argument().store_into(load_filter_filename).required();
    filter_module_command.add_subparser(filter_load);

    std::string unload_filter_name;
    argparse::ArgumentParser filter_unload("unload");
    filter_unload.add_description("Unload filter");
    filter_unload.add_argument().store_into(unload_filter_name).required();
    filter_module_command.add_subparser(filter_unload);

    argparse::ArgumentParser filter_module_status("status");
    filter_module_status.add_description("Filter module status");
    filter_module_command.add_subparser(filter_module_status);

    // Prometheus observer
    argparse::ArgumentParser prometheus_module_command("prometheus");
    prometheus_module_command.add_description("Prometheus observer module");

    argparse::ArgumentParser prometheus_start("start");
    prometheus_start.add_description("Start prometheus");
    prometheus_module_command.add_subparser(prometheus_start);

    argparse::ArgumentParser prometheus_stop("stop");
    prometheus_stop.add_description("Stop prometheus");
    prometheus_module_command.add_subparser(prometheus_stop);

    main_parser.add_subparser(ebpf_module_command);
    main_parser.add_subparser(kernel_module_command);
    main_parser.add_subparser(filter_module_command);
    main_parser.add_subparser(prometheus_module_command);

    try {
        main_parser.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << main_parser;
        return 1;
    }

    auto initialize_cmd = [&] -> std::optional<ant::ctl::InterfaceCommands> {
        if (main_parser.is_subcommand_used(ebpf_module_command)) {
            if (ebpf_module_command.is_subcommand_used(ebpf_load)) {
                return ant::ctl::LoadEbpfModule{};
            }
            if (ebpf_module_command.is_subcommand_used(ebpf_unload)) {
                return ant::ctl::UnloadEbpfModule{};
            }
            if (ebpf_module_command.is_subcommand_used(ebpf_status)) {
                return ant::ctl::StatusEbpfModule{};
            }
        } else if (main_parser.is_subcommand_used(kernel_module_command)) {
            if (kernel_module_command.is_subcommand_used(kernel_load)) {
                return ant::ctl::LoadKernelModule{};
            }
            if (kernel_module_command.is_subcommand_used(kernel_unload)) {
                return ant::ctl::UnloadKernelModule{};
            }
            if (kernel_module_command.is_subcommand_used(kernel_status)) {
                return ant::ctl::StatusKernelModule{};
            }
        } else if (main_parser.is_subcommand_used(filter_module_command)) {
            if (filter_module_command.is_subcommand_used(filter_load)) {
                return ant::ctl::FilterLoad{
                    .filter_filename = load_filter_filename
                };
            }
            if (filter_module_command.is_subcommand_used(filter_unload)) {
                return ant::ctl::FilterUnload{.filter_name = unload_filter_name};
            }
            if (filter_module_command.is_subcommand_used(filter_module_status)) {
                return ant::ctl::StatusFilterModule{};
            }
        } else if (main_parser.is_subcommand_used(prometheus_module_command)) {
            if (prometheus_module_command.is_subcommand_used(prometheus_start)) {
                return ant::ctl::StartPrometheus{};
            }
            if (prometheus_module_command.is_subcommand_used(prometheus_stop)) {
                return ant::ctl::StopPrometheus{};
            }
        }

        return std::nullopt;
    };

    std::optional<ant::ctl::InterfaceCommands> cmd = initialize_cmd();

    // Some Actions here
    const auto visit = ant::core::Overloaded{
        [](const ant::ctl::LoadEbpfModule &c) -> std::expected<void, ant::core::Error> {
            // something do
            return {};
        },
        [](const ant::ctl::UnloadEbpfModule &c) -> std::expected<void, ant::core::Error> {
            return {};
        },
        [](const ant::ctl::StatusEbpfModule &c) -> std::expected<void, ant::core::Error> {
            return {};
        },
        [](const ant::ctl::LoadKernelModule &c) -> std::expected<void, ant::core::Error> {
            std::string path = "../../modules/" ANT_KERNEL_MODULE_NAME ".ko";

            if (!std::filesystem::exists(path)) {
                ant::log::Debug(fmt::format("Path {} not exists!", path));
            }

            auto module = ant::kernel::KernelModule(
                ANT_KERNEL_MODULE_NAME, path);
            auto result = module.Load();
            if (!result) {
                return std::unexpected{result.error()};
            }

            return {};
        },
        [](const ant::ctl::UnloadKernelModule &c) -> std::expected<void, ant::core::Error> {
            auto module = ant::kernel::KernelModule(ANT_KERNEL_MODULE_NAME, "");

            auto result = module.Unload();
            if (!result) {
                return std::unexpected{result.error()};
            }

            return {};
        },
        [](const ant::ctl::StatusKernelModule &c) -> std::expected<void, ant::core::Error> {
            return {};
        },
        [](const ant::ctl::FilterLoad &c) -> std::expected<void, ant::core::Error> {
            return {};
        },
        [](const ant::ctl::FilterUnload &c) -> std::expected<void, ant::core::Error> {
            return {};
        },
        [](const ant::ctl::StatusFilterModule &c) -> std::expected<void, ant::core::Error> {
            return {};
        },
        [](const ant::ctl::StartPrometheus &c) -> std::expected<void, ant::core::Error> {
            return {};
        },
        [](const ant::ctl::StopPrometheus &c) -> std::expected<void, ant::core::Error> {
            return {};
        }
    };

    try {
        if (cmd.has_value()) {
            fmt::print("{}\n", cmd.value());

            if (auto result = std::visit(visit, cmd.value()); !result) {
                std::cout << fmt::format("Failed with error: {}\n", result.error()) << std::endl;
                return 1;
            }
        } else {
            std::cout << main_parser << std::endl;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Fatal: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
