#pragma once

#include "src/ports/Logger.h"

#include <memory>
#include <string_view>

namespace spdlog {
class logger;
}

namespace cw_api3d::adapters::driven::logging {

/// @brief Driven adapter wrapping the spdlog logging library.
/// Satisfies cw_api3d::ports::concepts::Logger and implements cw_api3d::ports::interfaces::ILogger.
class SpdLogLogger : public ports::interfaces::ILogger {
public:
    /// @brief Constructs adapter with a specific spdlog logger instance.
    /// If null, a default stdout color logger named "cw_api3d" will be initialized.
    explicit SpdLogLogger(std::shared_ptr<spdlog::logger> logger = nullptr) noexcept;

    ~SpdLogLogger() override = default;

    void set_level(ports::LogLevel level) noexcept override;
    [[nodiscard]] ports::LogLevel get_level() const noexcept override;
    [[nodiscard]] bool is_enabled(ports::LogLevel level) const noexcept override;

    void log(ports::LogLevel level, std::string_view message) noexcept override;

    /// @brief Provides access to the underlying spdlog::logger instance.
    [[nodiscard]] std::shared_ptr<spdlog::logger> underlying() const noexcept;

private:
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace cw_api3d::adapters::driven::logging
