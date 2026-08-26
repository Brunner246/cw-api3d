#pragma once

#include "src/ports/Logger.h"
#include <mutex>
#include <string>
#include <vector>

namespace cw_api3d::tests::doubles
{

  struct LogEntry
  {
    ports::LogLevel level;
    std::string message;

    bool operator==(const LogEntry& other) const = default;
  };

  class FakeLogger : public ports::ILogger
  {
  public:
    explicit FakeLogger(ports::LogLevel initialLevel = ports::LogLevel::Trace) noexcept
      : mCurrentLevel(initialLevel)
    {
    }

    void setLevel(ports::LogLevel level) noexcept override
    {
      std::lock_guard lock(mMutex);
      mCurrentLevel = level;
    }

    [[nodiscard]] ports::LogLevel getLevel() const noexcept override
    {
      std::lock_guard lock(mMutex);
      return mCurrentLevel;
    }

    [[nodiscard]] bool isEnabled(ports::LogLevel level) const noexcept override
    {
      std::lock_guard lock(mMutex);
      return static_cast<int>(level) >= static_cast<int>(mCurrentLevel) && mCurrentLevel != ports::LogLevel::Off;
    }

    void log(ports::LogLevel level, std::string_view message) noexcept override
    {
      if (!isEnabled(level))
      {
        return;
      }
      std::lock_guard lock(mMutex);
      mEntries.push_back(LogEntry{
        .level = level,
        .message = std::string(message)});
    }

    [[nodiscard]] std::vector<LogEntry> getEntries() const
    {
      std::lock_guard lock(mMutex);
      return mEntries;
    }

    void clear()
    {
      std::lock_guard lock(mMutex);
      mEntries.clear();
    }

    [[nodiscard]] bool hasMessage(ports::LogLevel level, std::string_view substring) const
    {
      std::lock_guard lock(mMutex);
      for (const auto& entry : mEntries)
      {
        if (entry.level == level && entry.message.find(substring) != std::string::npos)
        {
          return true;
        }
      }
      return false;
    }

    [[nodiscard]] std::size_t count() const
    {
      std::lock_guard lock(mMutex);
      return mEntries.size();
    }

  private:
    mutable std::mutex mMutex;
    ports::LogLevel mCurrentLevel{ports::LogLevel::Trace};
    std::vector<LogEntry> mEntries;
  };

} // namespace cw_api3d::tests::doubles
