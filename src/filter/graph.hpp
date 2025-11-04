#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <memory>
#include <utility>

#include <block.hpp>

namespace ant::filter {
    using DestPair = std::pair<int, int>; // {destination_sid, destination_port}
    using PortMap = std::unordered_map<int, std::vector<DestPair> >; // map of ports and their destination

    class Graph {
    public:
        std::vector<PortMap> blockOutputs; // vector by SID
        std::vector<std::unique_ptr<Block> > blocks; // vector by SID
    };
}
