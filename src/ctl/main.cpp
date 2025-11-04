#include <interface_commands.hpp>
#include <expected>
#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>
#include <boost/system/error_code.hpp>
#include <fmt/core.h>

int main(int argc, char *argv[]) {
    std::string input{};

    argparse::ArgumentParser parser("image-processing", "0.1.0", argparse::default_arguments::help);
    parser.add_description("Image processing program");

    argparse::ArgumentParser command_lab1("lab1");
    command_lab1.add_description("[circle-gray | blend]");

    argparse::ArgumentParser command_lab1_circle_gray("circle-gray");
    command_lab1_circle_gray.add_description("prints circle gray");

    argparse::ArgumentParser command_lab1_blending("blend");
    command_lab1_blending.add_description("blending two images");

    command_lab1.add_subparser(command_lab1_circle_gray);
    command_lab1.add_subparser(command_lab1_blending);
    parser.add_subparser(command_lab1);

    uint8_t n_levels;
    argparse::ArgumentParser command_lab2("lab2");
    command_lab2.add_description("starts Floyd-Stenberg algorithm");
    command_lab2.add_argument("-n", "--n-levels").required().store_into(n_levels).help("set level for scattering");

    parser.add_subparser(command_lab2);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    auto initialize_cmd = [&] -> std::optional<Command> {
        if (parser.is_subcommand_used(command_lab1)) {
            if (command_lab1.is_subcommand_used(command_lab1_circle_gray)) {
                return Command1{
                    .width = 512, .height = 512, .output_filename = input
                };
            }
            if (command_lab1.is_subcommand_used(command_lab1_blending)) {
                return Command2{
                    .first_filename = input
                };
            }
        } else if (parser.is_subcommand_used(command_lab2)) {
            return Command2{.first_filename = input};
        }

        return std::nullopt;
    };

    std::optional<Command> cmd = initialize_cmd();

    const auto v = overloaded{
        [](const Command1 &c) -> std::expected<void, boost::system::error_code> {
            fmt::print(
                "Something here\n",
                (std::filesystem::current_path() / c.output_filename).string(),
                c.width,
                c.height);

            return {};
        },
        [](const Command2 &c) -> std::expected<void, boost::system::error_code> {
            fmt::print("And something here\n", (std::filesystem::current_path() / c.first_filename).string());

            return {};
        }
    };

    try {
        if (cmd.has_value()) {
            if (auto result = std::visit(v, cmd.value()); !result) {
                fmt::print("Failed with error: {}\n", result.error().message());
                return 1;
            }
        } else {
            std::cout << parser << std::endl;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Fatal: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
