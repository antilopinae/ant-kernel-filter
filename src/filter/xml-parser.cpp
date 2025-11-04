#include <xml-parser.hpp>

#include <graph.hpp>
#include <block.hpp>
#include <fmt/format.h>

namespace ant::filter {
    auto XMLParser::Parse(const std::string &filename) const -> std::expected<Graph, core::Error> {
        Graph graph;
        tinyxml2::XMLDocument doc;

        if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
            return std::unexpected{core::Error(fmt::format("Can not open XML file {}", filename))};
        }

        auto system = doc.FirstChildElement("System");

        if (!system) {
            return std::unexpected{core::Error("Invalid Format: Missing <System> tag")};
        }

        int max_sid = -1;

        for (auto blk = system->FirstChildElement("Block"); blk;
             blk = blk->NextSiblingElement("Block")) {
            if (auto sid_str = blk->Attribute("SID"); sid_str) {
                max_sid = std::max(max_sid, std::stoi(sid_str));
            }
        }

        if (max_sid == -1) {
            return graph;
        }

        graph.blocks.resize(max_sid + 1);
        graph.blockOutputs.resize(max_sid + 1);

        for (auto blk = system->FirstChildElement("Block"); blk;
             blk = blk->NextSiblingElement("Block")) {
            auto sid_str = blk->Attribute("SID");
            auto type = blk->Attribute("BlockType");
            auto name = blk->Attribute("Name");

            if (!sid_str || !type || !name) {
                continue;
            }

            int sid = std::stoi(sid_str);

            graph.blocks[sid] = std::make_unique<Block>(sid, name, type);

            for (auto p = blk->FirstChildElement("P"); p; p = p->NextSiblingElement("P")) {
                if (auto p_name = p->Attribute("Name"); p_name && p->GetText()) {
                    graph.blocks[sid]->params[p_name] = p->GetText();
                }
            }

            if (auto port_tag = blk->FirstChildElement("Port"); port_tag) {
                if (auto port_name = FindPValue(port_tag, "Name"); port_name.has_value()) {
                    graph.blocks[sid]->params["PortName"] = port_name.value();
                }
            }
        }

        for (auto line = system->FirstChildElement("Line"); line;
             line = line->NextSiblingElement("Line")) {
            auto src_str = FindPValue(line, "Src");
            if (!src_str) {
                continue;
            }

            if (auto branch = line->FirstChildElement("Branch")) {
                for (auto br = branch; br; br = br->NextSiblingElement("Branch")) {
                    if (auto dst_str = FindPValue(br, "Dst"); dst_str.has_value()) {
                        if (auto result = CreateConnection(src_str.value(), dst_str.value(), graph); !result) {
                            return std::unexpected{result.error()};
                        }
                    }
                }
            } else {
                // simple line
                if (auto dst_str = FindPValue(line, "Dst"); dst_str.has_value()) {
                    if (auto result = CreateConnection(src_str.value(), dst_str.value(), graph); !result) {
                        return std::unexpected{result.error()};
                    }
                }
            }
        }

        return graph;
    }

    auto XMLParser::FindPValue(tinyxml2::XMLElement *parent,
                               const char *name) const -> std::expected<const char *, core::Error> {
        if (!parent) {
            return std::unexpected{core::Error("Invalid parent - nullptr")};
        }

        for (auto p = parent->FirstChildElement("P"); p; p = p->NextSiblingElement("P")) {
            if (auto p_name = p->Attribute("Name"); p_name && strcmp(p_name, name) == 0) {
                return p->GetText();
            }
        }

        return std::unexpected{core::Error("Nothing was found")};
    }

    auto XMLParser::ParseEndpoint(const std::string &text) const -> std::expected<Endpoint, core::Error> {
        Endpoint endpoint;

        size_t hash = text.find('#');
        size_t colon = text.find(':');

        if (hash == std::string::npos || colon == std::string::npos || hash > colon) {
            return std::unexpected{core::Error("Invalid Format: Src/Dst: " + text)};
        }

        endpoint.blockId = std::stoi(text.substr(0, hash));
        endpoint.port = std::stoi(text.substr(colon + 1));

        return endpoint;
    }

    auto XMLParser::CreateConnection(const char *srcStr, const char *dstStr,
                                     Graph &graph) const -> std::expected<void, core::Error> {
        int srcBlockId, dstBlockId, srcPort, dstPort;

        if (auto result = ParseEndpoint(srcStr); result.has_value()) {
            srcBlockId = result->blockId;
            srcPort = result->port;
        } else {
            return std::unexpected{result.error()};
        }

        if (auto result = ParseEndpoint(dstStr); result.has_value()) {
            dstBlockId = result->blockId;
            dstPort = result->port;
        } else {
            return std::unexpected{result.error()};
        }

        if (srcBlockId < graph.blocks.size() && dstBlockId < graph.blocks.size() && graph.blocks[srcBlockId] &&
            graph.blocks[dstBlockId]) {
            graph.blockOutputs[srcBlockId][srcPort].push_back({dstBlockId, dstPort});
            graph.blocks[dstBlockId]->inputs[dstPort] = srcBlockId;
        }

        return {};
    }
}
