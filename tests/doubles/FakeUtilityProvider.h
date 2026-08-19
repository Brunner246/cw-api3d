#pragma once

#include "src/ports/UtilityProvider.h"
#include <filesystem>
#include <mutex>
#include <optional>
#include <utility>

namespace cw_api3d::tests::doubles {

class FakeUtilityProvider : public ports::interfaces::IUtilityProvider {
public:
    explicit FakeUtilityProvider(std::optional<std::filesystem::path> plugin_path = std::nullopt) noexcept
        : plugin_path_(std::move(plugin_path)) {}

    void set_plugin_path(std::optional<std::filesystem::path> path) {
        std::lock_guard lock(mutex_);
        plugin_path_ = std::move(path);
    }

    [[nodiscard]] std::optional<std::filesystem::path> get_plugin_path() const noexcept override {
        std::lock_guard lock(mutex_);
        return plugin_path_;
    }

private:
    mutable std::mutex mutex_;
    std::optional<std::filesystem::path> plugin_path_;
};

} // namespace cw_api3d::tests::doubles
