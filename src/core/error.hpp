#pragma once

#include <string>

namespace ant::core {
    class Error {
    public:
        explicit Error(std::string message);

        [[nodiscard]] auto What() const -> std::string_view;

    private:
        std::string message_;
    };
}
