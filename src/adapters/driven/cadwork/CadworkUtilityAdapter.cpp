#include "CadworkUtilityAdapter.h"

#include <cwapi3d/ICwAPI3DString.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

#include <cctype>
#include <exception>
#include <string_view>
#include <utility>

namespace cw_api3d::adapters::driven::cadwork {

namespace {

[[nodiscard]] std::string_view trim_whitespace(std::string_view text) noexcept {
    constexpr auto is_space = [](char c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    };
    while (!text.empty() && is_space(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && is_space(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

} // namespace

CadworkUtilityAdapter::CadworkUtilityAdapter(
    CwAPI3D::Interfaces::ICwAPI3DUtilityController* utility_controller,
    ports::interfaces::LoggerPtr logger) noexcept
    : utility_controller_(utility_controller), logger_(std::move(logger)) {}

std::optional<std::filesystem::path> CadworkUtilityAdapter::get_plugin_path() const noexcept {
    if (!utility_controller_) {
        if (logger_) {
            logger_->warn("CadworkUtilityAdapter: ICwAPI3DUtilityController is null");
        }
        return std::nullopt;
    }

    try {
        auto* cw_str = utility_controller_->getPluginPath();
        if (!cw_str) {
            if (logger_) {
                logger_->warn("CadworkUtilityAdapter: getPluginPath() returned null pointer");
            }
            return std::nullopt;
        }

        // Invariant: NEVER call cw_str->destroy() — the ICwAPI3DString is host-owned and
        // destroying it corrupts the heap. Read the narrow data and leave memory management to the host.
        const auto* narrow = cw_str->narrowData();
        if (!narrow) {
            if (logger_) {
                logger_->warn("CadworkUtilityAdapter: narrowData() returned null pointer");
            }
            return std::nullopt;
        }

        const auto trimmed = trim_whitespace(std::string_view(narrow));
        if (trimmed.empty()) {
            if (logger_) {
                logger_->warn("CadworkUtilityAdapter: getPluginPath() returned empty string");
            }
            return std::nullopt;
        }

        return std::filesystem::path(trimmed);
    } catch (const std::exception& e) {
        if (logger_) {
            logger_->errorf("CadworkUtilityAdapter: Exception querying plugin path: {}", e.what());
        }
        return std::nullopt;
    } catch (...) {
        if (logger_) {
            logger_->error("CadworkUtilityAdapter: Unknown exception querying plugin path");
        }
        return std::nullopt;
    }
}

} // namespace cw_api3d::adapters::driven::cadwork
