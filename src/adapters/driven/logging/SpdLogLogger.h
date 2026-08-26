#pragma once

#include "src/ports/Logger.h"

#include <memory>
#include <string_view>

namespace spdlog
{
  class logger;
}

namespace cw_api3d::adapters::driven::logging
{

  /// @brief Driven adapter wrapping the spdlog logging library.
  /// Implements cw_api3d::ports::ILogger.
  class SpdLogLogger : public ports::ILogger
  {
  public:
    /// @brief Constructs adapter with a specific spdlog logger instance.
    /// If null, a default stdout color logger named "cw_api3d" will be initialized.
    explicit SpdLogLogger(std::shared_ptr<spdlog::logger> logger = nullptr) noexcept;

    ~SpdLogLogger() override = default;

    void setLevel(ports::LogLevel level) noexcept override;
    [[nodiscard]] ports::LogLevel getLevel() const noexcept override;
    [[nodiscard]] bool isEnabled(ports::LogLevel level) const noexcept override;

    void log(ports::LogLevel level, std::string_view message) noexcept override;

    /// @brief Provides access to the underlying spdlog::logger instance.
    [[nodiscard]] std::shared_ptr<spdlog::logger> underlying() const noexcept;

  private:
    std::shared_ptr<spdlog::logger> mLogger;
  };

} // namespace cw_api3d::adapters::driven::logging
