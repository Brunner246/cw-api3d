#include "SpdLogLogger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace cw_api3d::adapters::driven::logging {

namespace {

[[nodiscard]] constexpr spdlog::level::level_enum to_spdlog_level(ports::LogLevel level) noexcept {
    switch (level) {
        case ports::LogLevel::Trace:    return spdlog::level::trace;
        case ports::LogLevel::Debug:    return spdlog::level::debug;
        case ports::LogLevel::Info:     return spdlog::level::info;
        case ports::LogLevel::Warn:     return spdlog::level::warn;
        case ports::LogLevel::Error:    return spdlog::level::err;
        case ports::LogLevel::Critical: return spdlog::level::critical;
        case ports::LogLevel::Off:      return spdlog::level::off;
    }
    return spdlog::level::info;
}

[[nodiscard]] constexpr ports::LogLevel from_spdlog_level(spdlog::level::level_enum level) noexcept {
    switch (level) {
        case spdlog::level::trace:    return ports::LogLevel::Trace;
        case spdlog::level::debug:    return ports::LogLevel::Debug;
        case spdlog::level::info:     return ports::LogLevel::Info;
        case spdlog::level::warn:     return ports::LogLevel::Warn;
        case spdlog::level::err:      return ports::LogLevel::Error;
        case spdlog::level::critical: return ports::LogLevel::Critical;
        case spdlog::level::off:      return ports::LogLevel::Off;
        case spdlog::level::n_levels: return ports::LogLevel::Off;
    }
    return ports::LogLevel::Info;
}

} // namespace

SpdLogLogger::SpdLogLogger(std::shared_ptr<spdlog::logger> logger) noexcept
    : logger_(std::move(logger)) {
    if (!logger_) {
        logger_ = spdlog::get("cw_api3d");
        if (!logger_) {
            try {
                logger_ = spdlog::stdout_color_mt("cw_api3d");
            } catch (...) {
                logger_ = spdlog::default_logger();
            }
        }
    }
}

void SpdLogLogger::set_level(ports::LogLevel level) noexcept {
    if (logger_) {
        logger_->set_level(to_spdlog_level(level));
    }
}

ports::LogLevel SpdLogLogger::get_level() const noexcept {
    if (!logger_) {
        return ports::LogLevel::Off;
    }
    return from_spdlog_level(logger_->level());
}

bool SpdLogLogger::is_enabled(ports::LogLevel level) const noexcept {
    if (!logger_) {
        return false;
    }
    return logger_->should_log(to_spdlog_level(level));
}

void SpdLogLogger::log(ports::LogLevel level, std::string_view message) noexcept {
    if (!logger_) {
        return;
    }
    logger_->log(to_spdlog_level(level), "{}", message);
}

std::shared_ptr<spdlog::logger> SpdLogLogger::underlying() const noexcept {
    return logger_;
}

} // namespace cw_api3d::adapters::driven::logging
