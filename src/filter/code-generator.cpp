#include <code-generator.hpp>

#include <iostream>
#include <deque>
#include <queue>
#include <graph.hpp>
#include <block.hpp>
#include <fstream>
#include <sstream>

namespace ant::filter {
    auto CodeGenerator::Generate(const Graph &graph, const std::string &out_filename) -> void {
        std::vector<int> sorted_block_ids = TopologicalSort(graph);
        std::vector<int> delay_blocks;

        bytecode_.reserve(graph.blocks.size() * 6);

        for (const auto &id: sorted_block_ids) {
            if (graph.blocks[id]->type == "UnitDelay") {
                delay_blocks.push_back(id);
            }
        }

        GenHeader();
        GenStruct(graph, sorted_block_ids);
        GenInit(graph, delay_blocks);
        GenStep(graph, sorted_block_ids, delay_blocks);
        GenExtPorts(graph);

        WriteToFile(out_filename);
    }

    auto CodeGenerator::TopologicalSort(const Graph &graph) const -> std::vector<int> {
        std::vector<int> sorted_order;
        std::unordered_map<int, int> in_degree; // a map for storing the "incoming degree" of each vertex
        // key: SID of the block, value: how many blocks depend on it
        std::queue<int> q; // queue for blocks that do not have raw dependencies

        // calculation of initial incoming degrees
        for (const auto &block: graph.blocks) {
            // skip the empty slots in the vector
            if (!block) {
                continue;
            }
            int id = block->id;

            in_degree[id] = (block->type == "UnitDelay") ? 0 : block->inputs.size();

            if (in_degree[id] == 0) {
                q.push(id);
            }
        }

        while (!q.empty()) {
            int id = q.front();
            // 1. take the finished block from the queue
            q.pop();

            // 2. add it to the result
            sorted_order.push_back(id);

            if (id < graph.blockOutputs.size()) {
                // 3. process all the blocks that depend on it
                const auto &port_map = graph.blockOutputs[id];

                for (const auto &[src_port, destinations]: port_map) {
                    for (const auto &dest_pair: destinations) {
                        int dst_id = dest_pair.first;

                        // 4. reduce the dependency counter of the neighbor
                        if ((--in_degree[dst_id]) == 0) {
                            q.push(dst_id);
                        }
                    }
                }
            }
        }

        // checking for cycles
        size_t total_blocks = 0;

        for (const auto &block: graph.blocks)
            if (block && block->type != "Outport") {
                total_blocks++;
            }

        if (sorted_order.size() < total_blocks) {
            throw std::runtime_error("The graph contains a cycle. Topological sorting is not possible.");
        }

        return sorted_order;
    }

    auto CodeGenerator::GenHeader() -> void {
        bytecode_.emplace_back("#include \"ant_run.h\"");
        bytecode_.emplace_back("");
        bytecode_.emplace_back("#include <math.h>");
        bytecode_.emplace_back("");
    }

    auto CodeGenerator::GenStruct(const Graph &graph, const std::vector<int> &sorted_block_ids) -> void {
        bytecode_.emplace_back("static struct");
        bytecode_.emplace_back("{");

        for (const auto &id: sorted_block_ids) {
            const auto &block = graph.blocks[id];
            if (block->type != "Outport") {
                bytecode_.emplace_back("    double " + block->name + ";");
            }
        }

        bytecode_.emplace_back("} ant;");
        bytecode_.emplace_back("");
    }

    auto CodeGenerator::GenInit(const Graph &graph, const std::vector<int> &delay_blocks) -> void {
        bytecode_.emplace_back("void ant_generated_init()");
        bytecode_.emplace_back("{");

        for (const auto &id: delay_blocks) {
            bytecode_.emplace_back("    ant." + graph.blocks[id]->name + " = 0.0;");
        }

        bytecode_.emplace_back("}");
        bytecode_.emplace_back("");
    }

    auto CodeGenerator::GenStep(const Graph &graph, const std::vector<int> &sorted_block_ids,
                                const std::vector<int> &delay_blocks) -> void {
        bytecode_.emplace_back("void ant_generated_step()");
        bytecode_.emplace_back("{");

        // 1. computing part
        for (const auto &id: sorted_block_ids) {
            const auto &block = graph.blocks[id];

            if (block->type == "Sum") {
                std::ostringstream expr_stream;
                std::string inputs_str = block->params.count("Inputs") ? block->params.at("Inputs") : "++";

                expr_stream << block->GetCInputVar(1, graph);

                for (size_t i = 1; i < inputs_str.length(); ++i) {
                    expr_stream << " " << inputs_str[i] << " " << block->GetCInputVar(i + 1, graph);
                }

                bytecode_.emplace_back("    ant." + block->name + " = " + expr_stream.str() + ";");
            } else if (block->type == "Gain") {
                std::string gain_val = block->params.count("Gain") ? block->params.at("Gain") : "1.0";
                bytecode_.emplace_back(
                    "    ant." + block->name + " = " + block->GetCInputVar(1, graph) + " * " + gain_val + ";");
            }
        }

        // 2. updating part
        bytecode_.emplace_back("");
        bytecode_.emplace_back("    // Update delay blocks state");

        for (const auto &id: delay_blocks) {
            const auto &block = graph.blocks[id];
            bytecode_.emplace_back("    ant." + block->name + " = " + block->GetCInputVar(1, graph) + ";");
        }

        bytecode_.emplace_back("}");
        bytecode_.emplace_back("");
    }

    auto CodeGenerator::GenExtPorts(const Graph &graph) -> void {
        std::vector<std::string> outport_lines, inport_lines;

        bytecode_.emplace_back("static const ant_ExtPort ext_ports[] = {");

        for (const auto &block: graph.blocks) {
            if (!block) {
                continue;
            }

            if (block->type == "Inport") {
                std::string port_name = block->params.count("PortName") ? block->params.at("PortName") : block->name;
                inport_lines.push_back("    { \"" + port_name + "\", &ant." + block->name + ", 1 },");
            } else if (block->type == "Outport") {
                auto in_it = block->inputs.find(1);

                if (in_it == block->inputs.end()) {
                    throw std::runtime_error("Outport '" + block->name + "' has no input connection.");
                }

                int src_block_id = in_it->second;

                if (src_block_id >= graph.blocks.size() || !graph.blocks[src_block_id]) {
                    throw std::runtime_error("Source block for Outport '" + block->name + "' not found.");
                }

                std::string source_var = graph.blocks[src_block_id]->name;

                outport_lines.push_back("    { \"" + block->name + "\", &ant." + source_var + ", 0 },");
            }
        }

        for (const auto &line: outport_lines) {
            bytecode_.push_back(line);
        }

        for (const auto &line: inport_lines) {
            bytecode_.push_back(line);
        }

        bytecode_.emplace_back("    { 0, 0, 0 }");
        bytecode_.emplace_back("};");
        bytecode_.emplace_back("");

        bytecode_.emplace_back("const ant_ExtPort* const generated_ext_ports = ext_ports;");
        bytecode_.emplace_back("const size_t generated_ext_ports_size = sizeof(ext_ports);");
    }

    auto CodeGenerator::WriteToFile(const std::string &fname) const -> void {
        std::ofstream out(fname);

        for (const auto &instr: bytecode_) {
            out << instr << '\n';
        }
        out.close();

        std::cout << "Generated file: " << fname << std::endl;
    }
}
