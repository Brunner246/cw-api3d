#pragma once

#include <filesystem>
#include <memory>
#include <optional>

namespace cw_api3d::ports
{

  struct IUtilityProvider
  {
    virtual ~IUtilityProvider() = default;

    [[nodiscard]] virtual std::optional<std::filesystem::path> getPluginPath() const noexcept = 0;
  };

  using UtilityProviderPtr = std::shared_ptr<IUtilityProvider>;

} // namespace cw_api3d::ports
