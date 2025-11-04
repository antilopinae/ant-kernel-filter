#include <filter.hpp>

#include <stdexcept>
#include <iostream>
#include <utility>
#include <functional>

namespace ant::filter {
    Filter::Filter(std::string name) : name_(std::move(name)) {
    }

    auto Filter::Create(const std::string &path,
                        const std::string &name) -> std::expected<Filter, core::Error> {
        Filter filter(name);

        try {
            auto result = filter.parser_.Parse(path);
            if (!result) {
                return std::unexpected{result.error()};
            }
            filter.code_generator_.Generate(result.value(), path);

            return filter;
        } catch (const std::exception &ex) {
            return std::unexpected{core::Error{ex.what()}};
        }
    }
}
