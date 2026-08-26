#pragma once

#include "src/adapters/driven/cadwork/HostContracts.h"
#include "src/application/ElementSnapshot.h"
#include "src/ports/ElementActivation.h"
#include "src/ports/Logger.h"

#include <exception>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace cw_api3d::adapters::driven::cadwork
{

  template<concepts::ElementActivationSource Host>
  class ElementActivationAdapter final : public ports::IElementActivation
  {
  public:
    explicit ElementActivationAdapter(
      Host* host = nullptr,
      ports::LoggerPtr logger = nullptr) noexcept
      : mHost(host)
      , mLogger(std::move(logger))
    {
    }

    ~ElementActivationAdapter() override = default;

    [[nodiscard]] std::expected<void, std::string> activate(
      const std::span<const application::ElementId> elementIds) override
    {
      if (!mHost)
      {
        constexpr std::string_view err = "ElementActivationAdapter: host activation is null";
        warn(err);
        return std::unexpected(std::string{err});
      }

      if (elementIds.empty())
      {
        return {};
      }

      try
      {
        auto* list = mHost->createEmptyElementIDList();
        if (!list)
        {
          constexpr std::string_view err = "ElementActivationAdapter: host returned null ID list";
          warn(err);
          return std::unexpected(std::string{err});
        }
        for (const auto id : elementIds)
        {
          list->append(id);
        }
        mHost->setActive(list);
        return {};
      }
      catch (const std::exception& e)
      {
        if (mLogger)
        {
          mLogger->errorf("ElementActivationAdapter: Exception activating elements: {}", e.what());
        }
        return std::unexpected(std::string{e.what()});
      }
      catch (...)
      {
        constexpr std::string_view err = "ElementActivationAdapter: Unknown exception activating elements";
        if (mLogger)
        {
          mLogger->error(err);
        }
        return std::unexpected(std::string{err});
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

    Host* mHost{nullptr};
    ports::LoggerPtr mLogger;
  };

} // namespace cw_api3d::adapters::driven::cadwork
