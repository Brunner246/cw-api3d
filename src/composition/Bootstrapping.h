#pragma once

#include "src/application/QueryPluginPathUseCase.h"
#include "src/ports/Logger.h"
#include "src/ports/UtilityProvider.h"

#include <expected>
#include <filesystem>
#include <memory>
#include <string>

namespace CwAPI3D
{
  namespace Interfaces
  {
    class ICwAPI3DControllerFactory;
  }
  using ControllerFactory = Interfaces::ICwAPI3DControllerFactory;
} // namespace CwAPI3D

namespace cw_api3d::composition
{

  /// @brief Composition root bootstrapper that wires adapters to the application use case.
  /// Encapsulates concrete dependencies and ensures no exceptions cross the C boundary into host.
  class PluginBootstrapper
  {
  public:
    /// @brief Constructs bootstrapper with explicit driven adapter ports (useful for test injection).
    explicit PluginBootstrapper(
      ports::interfaces::UtilityProviderPtr utilityProvider,
      ports::interfaces::LoggerPtr logger) noexcept;

    /// @brief Creates a production bootstrapper configured with concrete SpdLogLogger and CadworkUtilityAdapter.
    [[nodiscard]] static std::unique_ptr<PluginBootstrapper> createProduction(
      CwAPI3D::ControllerFactory* factory) noexcept;

    /// @brief Creates a bootstrapper with the production logger and log-level policy wired to a
    /// caller-supplied utility provider port. Lets tests exercise the real logger, use case and
    /// assembly without an SDK-shaped host double.
    [[nodiscard]] static std::unique_ptr<PluginBootstrapper> createProductionForUtilityProvider(
      ports::interfaces::UtilityProviderPtr utilityProvider) noexcept;

    /// @brief Executes the composition root workflow.
    [[nodiscard]] std::expected<std::filesystem::path, std::string> run() noexcept;

    [[nodiscard]] ports::interfaces::UtilityProviderPtr utilityProvider() const noexcept
    {
      return mUtilityProvider;
    }

    [[nodiscard]] ports::interfaces::LoggerPtr logger() const noexcept
    {
      return mLogger;
    }

  private:
    ports::interfaces::UtilityProviderPtr mUtilityProvider;
    ports::interfaces::LoggerPtr mLogger;
    application::QueryPluginPathUseCase mUseCase;
  };

  /// @brief C-safe initialization function called by plugin entry points.
  /// Wires adapters, executes use case, catches all exceptions, and returns true on success.
  [[nodiscard]] bool bootstrapPlugin(CwAPI3D::ControllerFactory* factory) noexcept;

} // namespace cw_api3d::composition
