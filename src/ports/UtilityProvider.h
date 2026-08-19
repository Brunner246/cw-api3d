#pragma once

#include <concepts>
#include <filesystem>
#include <memory>
#include <optional>

namespace cw_api3d::ports {

namespace interfaces {

struct IUtilityProvider {
    virtual ~IUtilityProvider() = default;

    [[nodiscard]] virtual std::optional<std::filesystem::path> get_plugin_path() const noexcept = 0;
};

using UtilityProviderPtr = std::shared_ptr<IUtilityProvider>;

} // namespace interfaces

namespace concepts {

template <typename T>
concept UtilityProvider = requires(const T& provider) {
    { provider.get_plugin_path() } -> std::same_as<std::optional<std::filesystem::path>>;
};

} // namespace concepts

} // namespace cw_api3d::ports
