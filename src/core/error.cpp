#include <error.hpp>

namespace ant::core {
    Error::Error(std::string message) : message_(std::move(message)) {
    }

    auto Error::What() const -> std::string_view {
        return message_;
    }
}
