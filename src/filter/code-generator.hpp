#pragma once

#include <string>
#include <vector>

namespace ant::filter {
    class Graph;

    class CodeGenerator {
    public:
        auto Generate(const Graph &graph, const std::string &out_filename) -> void;

    private:
        // Kahns algorithm
        auto TopologicalSort(const Graph &graph) const -> std::vector<int>;

        auto GenHeader() -> void;

        auto GenStruct(const Graph &graph, const std::vector<int> &sorted_block_ids) -> void;

        auto GenInit(const Graph &graph, const std::vector<int> &delay_blocks) -> void;

        auto GenStep(const Graph &graph, const std::vector<int> &sorted_block_ids,
                     const std::vector<int> &delay_blocks) -> void;

        auto GenExtPorts(const Graph &graph) -> void;

        auto WriteToFile(const std::string &fname) const -> void;

    private:
        std::vector<std::string> bytecode_;
    };
}
