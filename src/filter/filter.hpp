#pragma once

#include <expected>

#include <string>
#include <xml-parser.hpp>
#include <code-generator.hpp>
#include <filesystem>
#include <graph.hpp>

namespace ant::filter {
    class Filter {
    private:
        explicit Filter(std::string name);

    public:
        static auto Create(const std::string &path,
                           const std::string &name = "ant_generated.c") -> std::expected<Filter, core::Error>;

    private:
        Graph graph_;
        CodeGenerator code_generator_;
        std::string name_;
        XMLParser parser_;
    };
}
