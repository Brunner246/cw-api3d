#pragma once

#include "src/ports/UtilityProvider.h"
#include <filesystem>
#include <mutex>
#include <optional>
#include <utility>

namespace cw_api3d::tests::doubles
{

  class FakeUtilityProvider : public ports::interfaces::IUtilityProvider
  {
  public:
    explicit FakeUtilityProvider(std::optional<std::filesystem::path> pluginPath = std::nullopt) noexcept
      : mPluginPath(std::move(pluginPath))
    {
    }

    void setPluginPath(std::optional<std::filesystem::path> path)
    {
      std::lock_guard lock(mMutex);
      mPluginPath = std::move(path);
    }

    [[nodiscard]] std::optional<std::filesystem::path> getPluginPath() const noexcept override
    {
      std::lock_guard lock(mMutex);
      return mPluginPath;
    }

  private:
    mutable std::mutex mMutex;
    std::optional<std::filesystem::path> mPluginPath;
  };

} // namespace cw_api3d::tests::doubles
