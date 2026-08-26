#pragma once

#include "src/adapters/driven/cadwork/HostContracts.h"
#include "src/ports/Logger.h"
#include "src/ports/UtilityProvider.h"

#include <exception>
#include <filesystem>
#include <optional>
#include <string_view>
#include <utility>

namespace cw_api3d::adapters::driven::cadwork
{

  /// @brief Driven adapter translating a host utility controller into ports::IUtilityProvider.
  /// Constrained on concepts::PluginPathSource rather than a concrete SDK interface, so test doubles
  /// need only the two methods actually used instead of the whole host interface.
  /// Safely extracts plugin path string data without calling destroy() on host-owned strings.
  template<concepts::PluginPathSource Controller>
  class UtilityControllerAdapter final : public ports::IUtilityProvider
  {
  public:
    explicit UtilityControllerAdapter(
      Controller* utilityController = nullptr,
      ports::LoggerPtr logger = nullptr) noexcept
      : mUtilityController(utilityController)
      , mLogger(std::move(logger))
    {
    }

    ~UtilityControllerAdapter() override = default;

    [[nodiscard]] std::optional<std::filesystem::path> getPluginPath() const noexcept override
    {
      if (!mUtilityController)
      {
        warn("UtilityControllerAdapter: host utility controller is null");
        return std::nullopt;
      }

      try
      {
        auto* hostString = mUtilityController->getPluginPath();
        if (!hostString)
        {
          warn("UtilityControllerAdapter: getPluginPath() returned null pointer");
          return std::nullopt;
        }

        // Invariant: NEVER call hostString->destroy() — the host string is host-owned and
        // destroying it corrupts the heap. Read the narrow data and leave memory management to the host.
        const auto* narrow = hostString->narrowData();
        if (!narrow)
        {
          warn("UtilityControllerAdapter: narrowData() returned null pointer");
          return std::nullopt;
        }

        const auto trimmed = detail::trimWhitespace(std::string_view(narrow));
        if (trimmed.empty())
        {
          warn("UtilityControllerAdapter: getPluginPath() returned empty string");
          return std::nullopt;
        }

        return std::filesystem::path(trimmed);
      }
      catch (const std::exception& e)
      {
        if (mLogger)
        {
          mLogger->errorf("UtilityControllerAdapter: Exception querying plugin path: {}", e.what());
        }
        return std::nullopt;
      }
      catch (...)
      {
        if (mLogger)
        {
          mLogger->error("UtilityControllerAdapter: Unknown exception querying plugin path");
        }
        return std::nullopt;
      }
    }

  private:
    void warn(const std::string_view message) const noexcept
    {
      if (mLogger)
      {
        mLogger->warn(message);
      }
    }

    Controller* mUtilityController{nullptr};
    ports::LoggerPtr mLogger;
  };

} // namespace cw_api3d::adapters::driven::cadwork
