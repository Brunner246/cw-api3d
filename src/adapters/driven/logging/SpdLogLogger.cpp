#include "SpdLogLogger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace cw_api3d::adapters::driven::logging
{

  namespace
  {

    [[nodiscard]] constexpr spdlog::level::level_enum toSpdlogLevel(ports::LogLevel level) noexcept
    {
      switch (level)
      {
        case ports::LogLevel::Trace:
          return spdlog::level::trace;
        case ports::LogLevel::Debug:
          return spdlog::level::debug;
        case ports::LogLevel::Info:
          return spdlog::level::info;
        case ports::LogLevel::Warn:
          return spdlog::level::warn;
        case ports::LogLevel::Error:
          return spdlog::level::err;
        case ports::LogLevel::Critical:
          return spdlog::level::critical;
        case ports::LogLevel::Off:
          return spdlog::level::off;
      }
      return spdlog::level::info;
    }

    [[nodiscard]] constexpr ports::LogLevel fromSpdlogLevel(spdlog::level::level_enum level) noexcept
    {
      switch (level)
      {
        case spdlog::level::trace:
          return ports::LogLevel::Trace;
        case spdlog::level::debug:
          return ports::LogLevel::Debug;
        case spdlog::level::info:
          return ports::LogLevel::Info;
        case spdlog::level::warn:
          return ports::LogLevel::Warn;
        case spdlog::level::err:
          return ports::LogLevel::Error;
        case spdlog::level::critical:
          return ports::LogLevel::Critical;
        case spdlog::level::off:
          return ports::LogLevel::Off;
        case spdlog::level::n_levels:
          return ports::LogLevel::Off;
      }
      return ports::LogLevel::Info;
    }

  } // namespace

  SpdLogLogger::SpdLogLogger(std::shared_ptr<spdlog::logger> logger) noexcept
    : mLogger(std::move(logger))
  {
    if (!mLogger)
    {
      mLogger = spdlog::get("cw_api3d");
      if (!mLogger)
      {
        try
        {
          mLogger = spdlog::stdout_color_mt("cw_api3d");
        }
        catch (...)
        {
          mLogger = spdlog::default_logger();
        }
      }
    }
  }

  void SpdLogLogger::setLevel(ports::LogLevel level) noexcept
  {
    if (mLogger)
    {
      mLogger->set_level(toSpdlogLevel(level));
    }
  }

  ports::LogLevel SpdLogLogger::getLevel() const noexcept
  {
    if (!mLogger)
    {
      return ports::LogLevel::Off;
    }
    return fromSpdlogLevel(mLogger->level());
  }

  bool SpdLogLogger::isEnabled(ports::LogLevel level) const noexcept
  {
    if (!mLogger)
    {
      return false;
    }
    return mLogger->should_log(toSpdlogLevel(level));
  }

  void SpdLogLogger::log(ports::LogLevel level, std::string_view message) noexcept
  {
    if (!mLogger)
    {
      return;
    }
    mLogger->log(toSpdlogLevel(level), "{}", message);
  }

  std::shared_ptr<spdlog::logger> SpdLogLogger::underlying() const noexcept
  {
    return mLogger;
  }

} // namespace cw_api3d::adapters::driven::logging
