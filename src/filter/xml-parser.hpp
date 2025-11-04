#pragma once

#include <tinyxml/tinyxml2.h>
#include <endpoint.hpp>
#include <expected>
#include <core/error.hpp>
#include <string>

namespace ant::filter {
    class Graph;

    class XMLParser {
    public:
        [[nodiscard]] auto Parse(const std::string &filename) const -> std::expected<Graph, core::Error>;

    private:
        auto CreateConnection(const char *srcStr, const char *dstStr,
                              Graph &graph) const -> std::expected<void, core::Error>;

        [[nodiscard]] auto ParseEndpoint(const std::string &text) const -> std::expected<Endpoint, core::Error>;

        [[nodiscard]] auto FindPValue(tinyxml2::XMLElement *parent,
                                      const char *name) const -> std::expected<const char *, core::Error>;
    };
}
