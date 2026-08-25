#pragma once

#include "src/ports/ElementCatalog.h"

#include <expected>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace cw_api3d::tests::doubles
{

  class FakeElementCatalog : public ports::interfaces::IElementCatalog
  {
  public:
    void setActive(application::ElementSnapshot snapshot)
    {
      std::lock_guard lock(mMutex);
      mActive = std::move(snapshot);
    }

    void setAll(application::ElementSnapshot snapshot)
    {
      std::lock_guard lock(mMutex);
      mAll = std::move(snapshot);
    }

    void setFailure(std::string message)
    {
      std::lock_guard lock(mMutex);
      mFailure = std::move(message);
    }

    [[nodiscard]] std::expected<application::ElementSnapshot, std::string> fetch(
      const application::ElementUniverse universe) const override
    {
      std::lock_guard lock(mMutex);
      ++mCallCount;
      if (mFailure.has_value())
      {
        return std::unexpected(*mFailure);
      }
      switch (universe)
      {
        case application::ElementUniverse::Active:
          return mActive;
        case application::ElementUniverse::All:
          return mAll;
      }
      return std::unexpected("Unknown element universe");
    }

    [[nodiscard]] int callCount() const
    {
      std::lock_guard lock(mMutex);
      return mCallCount;
    }

  private:
    mutable std::mutex mMutex;
    application::ElementSnapshot mActive;
    application::ElementSnapshot mAll;
    std::optional<std::string> mFailure;
    mutable int mCallCount{0};
  };

} // namespace cw_api3d::tests::doubles
