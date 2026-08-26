#pragma once

#include "src/ports/Logger.h"
#include "src/ports/UtilityProvider.h"

#include <expected>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace cw_api3d::application
{

  /// @brief Driving port interface for querying the plugin path.
  struct IQueryPluginPathUseCase
  {
    virtual ~IQueryPluginPathUseCase() = default;

    [[nodiscard]] virtual std::expected<std::filesystem::path, std::string> execute() = 0;
  };

  using QueryPluginPathUseCasePtr = std::shared_ptr<IQueryPluginPathUseCase>;

  /// @brief Application use case orchestrating plugin path query and diagnostic logging.
  class QueryPluginPathUseCase : public IQueryPluginPathUseCase
  {
  public:
    /// @brief Constructor accepting driven port references.
    explicit QueryPluginPathUseCase(
      ports::IUtilityProvider& utilityProvider,
      ports::ILogger& logger) noexcept
      : mUtilityProvider(&utilityProvider)
      , mLogger(&logger)
    {
    }

    /// @brief Constructor accepting driven port shared ownership pointers.
    explicit QueryPluginPathUseCase(
      ports::UtilityProviderPtr utilityProvider,
      ports::LoggerPtr logger) noexcept
      : mUtilityProviderPtr(std::move(utilityProvider))
      , mLoggerPtr(std::move(logger))
      , mUtilityProvider(mUtilityProviderPtr.get())
      , mLogger(mLoggerPtr.get())
    {
    }

    /// @brief Executes the use case orchestrating plugin path query and logging.
    [[nodiscard]] std::expected<std::filesystem::path, std::string> execute() override
    {
      if (!mUtilityProvider || !mLogger)
      {
        return std::unexpected("Utility provider or logger dependency is uninitialized");
      }

      mLogger->trace("Querying plugin path from host utility provider...");

      const auto pathOpt = mUtilityProvider->getPluginPath();
      if (!pathOpt.has_value())
      {
        constexpr std::string_view errMsg = "Plugin path is not available from utility provider";
        mLogger->warn(errMsg);
        return std::unexpected(std::string(errMsg));
      }

      const auto& path = *pathOpt;
      mLogger->info(std::format("Retrieved plugin path: {}", path.string()));
      return path;
    }

  private:
    ports::UtilityProviderPtr mUtilityProviderPtr;
    ports::LoggerPtr mLoggerPtr;
    ports::IUtilityProvider* mUtilityProvider{nullptr};
    ports::ILogger* mLogger{nullptr};
  };

} // namespace cw_api3d::application
