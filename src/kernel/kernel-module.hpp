#pragma once

#include <cstdint>
#include <string>
#include <expected>
#include <core/error.hpp>

namespace ant::kernel {
    namespace detail {
        static auto CreateMap() -> std::expected<void, core::Error>;

        static auto CloseMap() -> std::expected<void, core::Error>;

        static auto ReadMap() -> std::expected<void, core::Error>;
    }

    enum class KernelModuleStatus: uint8_t {
        kNotLoaded = 0,
        kLoaded = 1,
    };

    class KernelModule {
    public:
        KernelModule(std::string module_name, std::string module_path) noexcept;

        auto Load() -> std::expected<KernelModuleStatus, core::Error>;

        auto Unload() -> std::expected<KernelModuleStatus, core::Error>;

        [[nodiscard]] auto GetPath() const noexcept -> std::string {
            return module_path_;
        }

        [[nodiscard]] auto GetName() const noexcept -> std::string {
            return module_name_;
        }

        [[nodiscard]] auto GetStatus() const noexcept -> KernelModuleStatus {
            return status_;
        }

    private:
        std::string module_path_;
        std::string module_name_;
        KernelModuleStatus status_;
    };
}
