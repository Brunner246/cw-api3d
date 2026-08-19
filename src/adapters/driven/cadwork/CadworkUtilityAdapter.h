#pragma once

#include "src/ports/Logger.h"
#include "src/ports/UtilityProvider.h"

#include <filesystem>
#include <optional>

namespace CwAPI3D::Interfaces {
class ICwAPI3DUtilityController;
}

namespace cw_api3d::adapters::driven::cadwork {

/// @brief Driven adapter wrapping CwAPI3D::Interfaces::ICwAPI3DUtilityController.
/// Satisfies cw_api3d::ports::concepts::UtilityProvider and implements cw_api3d::ports::interfaces::IUtilityProvider.
/// Safely extracts plugin path string data without calling destroy() on host-owned strings.
class CadworkUtilityAdapter : public ports::interfaces::IUtilityProvider {
public:
    explicit CadworkUtilityAdapter(
        CwAPI3D::Interfaces::ICwAPI3DUtilityController* utility_controller = nullptr,
        ports::interfaces::LoggerPtr logger = nullptr) noexcept;

    ~CadworkUtilityAdapter() override = default;

    [[nodiscard]] std::optional<std::filesystem::path> get_plugin_path() const noexcept override;

private:
    CwAPI3D::Interfaces::ICwAPI3DUtilityController* utility_controller_{nullptr};
    ports::interfaces::LoggerPtr logger_;
};

} // namespace cw_api3d::adapters::driven::cadwork
