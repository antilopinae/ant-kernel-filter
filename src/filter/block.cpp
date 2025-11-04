#include <block.hpp>
#include <algorithm>
#include <stdexcept>
#include <graph.hpp>

namespace ant::filter {
    Block::Block(int id, const std::string &name, const std::string &type) : id(id), name(SanitizeName(name)),
                                                                             type(type) {
    }

    auto Block::GetCInputVar(int port, const Graph &graph) const -> std::string {
        auto it = inputs.find(port);
        if (it == inputs.end()) {
            throw std::runtime_error(
                "Block '" + name + "' (SID: " + std::to_string(id) + ") has no connection to input port " +
                std::to_string(port));
        }

        int sourceBlockId = it->second; // getting the source block ID
        if (sourceBlockId >= graph.blocks.size() || !graph.blocks[sourceBlockId]) {
            throw std::runtime_error("Source block with SID '" + std::to_string(sourceBlockId) + "' not found.");
        }
        return "ant." + graph.blocks[sourceBlockId]->name;
    }

    auto Block::SanitizeName(const std::string &name) -> std::string {
        std::string sanitized = name;
        std::replace(sanitized.begin(), sanitized.end(), ' ', '_');
        return sanitized;
    }
}
