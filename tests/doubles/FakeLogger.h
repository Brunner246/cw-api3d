#pragma once

#include "src/ports/Logger.h"
#include <mutex>
#include <string>
#include <vector>

namespace cw_api3d::tests::doubles {

struct LogEntry {
    ports::LogLevel level;
    std::string message;

    bool operator==(const LogEntry& other) const = default;
};

class FakeLogger : public ports::interfaces::ILogger {
public:
    explicit FakeLogger(ports::LogLevel initial_level = ports::LogLevel::Trace) noexcept
        : current_level_(initial_level) {}

    void set_level(ports::LogLevel level) noexcept override {
        std::lock_guard lock(mutex_);
        current_level_ = level;
    }

    [[nodiscard]] ports::LogLevel get_level() const noexcept override {
        std::lock_guard lock(mutex_);
        return current_level_;
    }

    [[nodiscard]] bool is_enabled(ports::LogLevel level) const noexcept override {
        std::lock_guard lock(mutex_);
        return static_cast<int>(level) >= static_cast<int>(current_level_) && current_level_ != ports::LogLevel::Off;
    }

    void log(ports::LogLevel level, std::string_view message) noexcept override {
        if (!is_enabled(level)) {
            return;
        }
        std::lock_guard lock(mutex_);
        entries_.push_back(LogEntry{
            .level = level,
            .message = std::string(message)
        });
    }

    [[nodiscard]] std::vector<LogEntry> get_entries() const {
        std::lock_guard lock(mutex_);
        return entries_;
    }

    void clear() {
        std::lock_guard lock(mutex_);
        entries_.clear();
    }

    [[nodiscard]] bool has_message(ports::LogLevel level, std::string_view substring) const {
        std::lock_guard lock(mutex_);
        for (const auto& entry : entries_) {
            if (entry.level == level && entry.message.find(substring) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::size_t count() const {
        std::lock_guard lock(mutex_);
        return entries_.size();
    }

private:
    mutable std::mutex mutex_;
    ports::LogLevel current_level_{ports::LogLevel::Trace};
    std::vector<LogEntry> entries_;
};

} // namespace cw_api3d::tests::doubles
