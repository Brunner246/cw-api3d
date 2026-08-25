#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <concepts>
#include <format>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace cw_api3d::ports
{

  enum class LogLevel
  {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off
  };

  [[nodiscard]] constexpr std::string_view toString(const LogLevel level) noexcept
  {
    switch (level)
    {
      case LogLevel::Trace:
        return "Trace";
      case LogLevel::Debug:
        return "Debug";
      case LogLevel::Info:
        return "Info";
      case LogLevel::Warn:
        return "Warn";
      case LogLevel::Error:
        return "Error";
      case LogLevel::Critical:
        return "Critical";
      case LogLevel::Off:
        return "Off";
    }
    return "Unknown";
  }

  [[nodiscard]] inline std::optional<LogLevel> parseLogLevel(std::string_view text) noexcept
  {
    constexpr auto isSpace = [](const char c) noexcept {
      return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    };
    while (!text.empty() && isSpace(text.front()))
    {
      text.remove_prefix(1);
    }
    while (!text.empty() && isSpace(text.back()))
    {
      text.remove_suffix(1);
    }
    if (text.empty())
    {
      return std::nullopt;
    }

    constexpr std::array<std::pair<std::string_view, LogLevel>, 9> names{{
      {"trace", LogLevel::Trace},
      {"debug", LogLevel::Debug},
      {"info", LogLevel::Info},
      {"warn", LogLevel::Warn},
      {"warning", LogLevel::Warn},
      {"error", LogLevel::Error},
      {"err", LogLevel::Error},
      {"critical", LogLevel::Critical},
      {"off", LogLevel::Off},
    }};

    for (const auto& [name, level] : names)
    {
      if (name.size() != text.size())
      {
        continue;
      }
      const bool matches = std::equal(name.begin(), name.end(), text.begin(),
                                      [](const char lhs, const char rhs) noexcept {
                                        return lhs == static_cast<char>(std::tolower(static_cast<unsigned char>(rhs)));
                                      });
      if (matches)
      {
        return level;
      }
    }
    return std::nullopt;
  }

  namespace interfaces
  {

    struct ILogger
    {
      virtual ~ILogger() = default;

      virtual void setLevel(LogLevel level) noexcept = 0;
      [[nodiscard]] virtual LogLevel getLevel() const noexcept = 0;
      [[nodiscard]] virtual bool isEnabled(LogLevel level) const noexcept = 0;

      virtual void log(LogLevel level, std::string_view message) noexcept = 0;

      void trace(const std::string_view message) noexcept
      {
        log(LogLevel::Trace, message);
      }
      void debug(const std::string_view message) noexcept
      {
        log(LogLevel::Debug, message);
      }
      void info(const std::string_view message) noexcept
      {
        log(LogLevel::Info, message);
      }
      void warn(const std::string_view message) noexcept
      {
        log(LogLevel::Warn, message);
      }
      void error(const std::string_view message) noexcept
      {
        log(LogLevel::Error, message);
      }
      void critical(const std::string_view message) noexcept
      {
        log(LogLevel::Critical, message);
      }

      template<typename... Args>
      void logf(LogLevel level, std::format_string<Args...> fmt, Args&&... args) noexcept
      {
        if (!isEnabled(level))
        {
          return;
        }
        try
        {
          auto msg = std::format(fmt, std::forward<Args>(args)...);
          log(level, msg);
        }
        catch (...)
        {
          log(level, fmt.get());
        }
      }

      template<typename... Args>
      void tracef(std::format_string<Args...> fmt, Args&&... args) noexcept
      {
        logf(LogLevel::Trace, fmt, std::forward<Args>(args)...);
      }
      template<typename... Args>
      void debugf(std::format_string<Args...> fmt, Args&&... args) noexcept
      {
        logf(LogLevel::Debug, fmt, std::forward<Args>(args)...);
      }
      template<typename... Args>
      void infof(std::format_string<Args...> fmt, Args&&... args) noexcept
      {
        logf(LogLevel::Info, fmt, std::forward<Args>(args)...);
      }
      template<typename... Args>
      void warnf(std::format_string<Args...> fmt, Args&&... args) noexcept
      {
        logf(LogLevel::Warn, fmt, std::forward<Args>(args)...);
      }
      template<typename... Args>
      void errorf(std::format_string<Args...> fmt, Args&&... args) noexcept
      {
        logf(LogLevel::Error, fmt, std::forward<Args>(args)...);
      }
      template<typename... Args>
      void criticalf(std::format_string<Args...> fmt, Args&&... args) noexcept
      {
        logf(LogLevel::Critical, fmt, std::forward<Args>(args)...);
      }
    };

    using LoggerPtr = std::shared_ptr<ILogger>;

  } // namespace interfaces

  namespace concepts
  {

    template<typename T>
    concept Logger = requires(T& logger, const T& constLogger, LogLevel level, std::string_view message) {
      { constLogger.getLevel() } -> std::same_as<LogLevel>;
      { constLogger.isEnabled(level) } -> std::convertible_to<bool>;
      { logger.log(level, message) };
      { logger.info(message) };
      { logger.warn(message) };
      { logger.error(message) };
      { logger.debug(message) };
      { logger.trace(message) };
      { logger.critical(message) };
    };

  } // namespace concepts

} // namespace cw_api3d::ports
