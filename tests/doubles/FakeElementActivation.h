#pragma once

#include "src/ports/ElementActivation.h"

#include <expected>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace cw_api3d::tests::doubles
{

  class FakeElementActivation : public ports::IElementActivation
  {
  public:
    void setFailure(std::string message)
    {
      std::lock_guard lock(mMutex);
      mFailure = std::move(message);
    }

    [[nodiscard]] std::expected<void, std::string> activate(
      const std::span<const application::ElementId> elementIds) override
    {
      std::lock_guard lock(mMutex);
      ++mCallCount;
      mLastIds.assign(elementIds.begin(), elementIds.end());
      if (mFailure.has_value())
      {
        return std::unexpected(*mFailure);
      }
      return {};
    }

    [[nodiscard]] std::vector<application::ElementId> lastIds() const
    {
      std::lock_guard lock(mMutex);
      return mLastIds;
    }

    [[nodiscard]] int callCount() const
    {
      std::lock_guard lock(mMutex);
      return mCallCount;
    }

  private:
    mutable std::mutex mMutex;
    std::vector<application::ElementId> mLastIds;
    std::optional<std::string> mFailure;
    int mCallCount{0};
  };

} // namespace cw_api3d::tests::doubles
