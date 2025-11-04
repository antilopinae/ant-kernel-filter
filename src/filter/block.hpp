#pragma once

#include <string>
#include <unordered_map>

namespace ant::filter {
    class Graph;

    class Block {
    public:
        std::unordered_map<std::string, std::string> params;
        std::unordered_map<int, int> inputs; // {my port, source block sid}
        std::string name;
        std::string type;
        int id;

        Block(int id, const std::string &name, const std::string &type);

        auto GetCInputVar(int port, const Graph &graph) const -> std::string;

    private:
        auto SanitizeName(const std::string &name) -> std::string;
    };
}
