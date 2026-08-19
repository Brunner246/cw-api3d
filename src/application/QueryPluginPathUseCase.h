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

namespace cw_api3d::application {

/// @brief Driving port interface for querying the plugin path.
struct IQueryPluginPathUseCase {
    virtual ~IQueryPluginPathUseCase() = default;

    [[nodiscard]] virtual std::expected<std::filesystem::path, std::string> execute() = 0;
};

using QueryPluginPathUseCasePtr = std::shared_ptr<IQueryPluginPathUseCase>;

/// @brief Application use case orchestrating plugin path query and diagnostic logging.
/// Demonstrates dual polymorphism: static concept constraints and dynamic virtual interface dispatch.
class QueryPluginPathUseCase : public IQueryPluginPathUseCase {
public:
    /// @brief Dynamic polymorphism constructor with raw reference dependencies.
    explicit QueryPluginPathUseCase(
        ports::interfaces::IUtilityProvider& utility_provider,
        ports::interfaces::ILogger& logger) noexcept
        : utility_provider_(&utility_provider), logger_(&logger) {}

    /// @brief Dynamic polymorphism constructor with shared ownership pointers.
    explicit QueryPluginPathUseCase(
        ports::interfaces::UtilityProviderPtr utility_provider,
        ports::interfaces::LoggerPtr logger) noexcept
        : utility_provider_ptr_(std::move(utility_provider)),
          logger_ptr_(std::move(logger)),
          utility_provider_(utility_provider_ptr_.get()),
          logger_(logger_ptr_.get()) {}

    /// @brief Static polymorphism generic executor constrained by C++20 concepts.
    template <ports::concepts::UtilityProvider U, ports::concepts::Logger L>
    [[nodiscard]] static std::expected<std::filesystem::path, std::string> execute_static(
        const U& utility_provider, L& logger) {
        logger.trace("Querying plugin path from host utility provider...");

        const auto path_opt = utility_provider.get_plugin_path();
        if (!path_opt.has_value()) {
            constexpr std::string_view err_msg = "Plugin path is not available from utility provider";
            logger.warn(err_msg);
            return std::unexpected(std::string(err_msg));
        }

        const auto& path = *path_opt;
        logger.info(std::format("Retrieved plugin path: {}", path.string()));
        return path;
    }

    /// @brief Executes the use case via dynamic interface dispatch.
    [[nodiscard]] std::expected<std::filesystem::path, std::string> execute() override {
        if (!utility_provider_ || !logger_) {
            return std::unexpected("Utility provider or logger dependency is uninitialized");
        }
        return execute_static(*utility_provider_, *logger_);
    }

private:
    ports::interfaces::UtilityProviderPtr utility_provider_ptr_;
    ports::interfaces::LoggerPtr logger_ptr_;
    ports::interfaces::IUtilityProvider* utility_provider_{nullptr};
    ports::interfaces::ILogger* logger_{nullptr};
};

} // namespace cw_api3d::application
