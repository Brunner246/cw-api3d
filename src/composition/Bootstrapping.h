#pragma once

#include "src/application/QueryPluginPathUseCase.h"
#include "src/ports/Logger.h"
#include "src/ports/UtilityProvider.h"

#include <expected>
#include <filesystem>
#include <memory>
#include <string>

namespace CwAPI3D {
namespace Interfaces {
class ICwAPI3DControllerFactory;
}
using ControllerFactory = Interfaces::ICwAPI3DControllerFactory;
} // namespace CwAPI3D

namespace cw_api3d::composition {

/// @brief Composition root bootstrapper that wires adapters to the application use case.
/// Encapsulates concrete dependencies and ensures no exceptions cross the C boundary into host.
class PluginBootstrapper {
public:
    /// @brief Constructs bootstrapper with explicit driven adapter ports (useful for test injection).
    explicit PluginBootstrapper(
        ports::interfaces::UtilityProviderPtr utility_provider,
        ports::interfaces::LoggerPtr logger) noexcept;

    /// @brief Creates a production bootstrapper configured with concrete SpdLogLogger and CadworkUtilityAdapter.
    [[nodiscard]] static std::unique_ptr<PluginBootstrapper> create_production(
        CwAPI3D::ControllerFactory* factory) noexcept;

    /// @brief Executes the composition root workflow.
    [[nodiscard]] std::expected<std::filesystem::path, std::string> run() noexcept;

    [[nodiscard]] ports::interfaces::UtilityProviderPtr utility_provider() const noexcept {
        return utility_provider_;
    }

    [[nodiscard]] ports::interfaces::LoggerPtr logger() const noexcept {
        return logger_;
    }

private:
    ports::interfaces::UtilityProviderPtr utility_provider_;
    ports::interfaces::LoggerPtr logger_;
    application::QueryPluginPathUseCase use_case_;
};

/// @brief C-safe initialization function called by plugin entry points.
/// Wires adapters, executes use case, catches all exceptions, and returns true on success.
[[nodiscard]] bool bootstrap_plugin(CwAPI3D::ControllerFactory* factory) noexcept;

} // namespace cw_api3d::composition
