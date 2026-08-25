#pragma once

#include "src/application/ElementSnapshot.h"
#include "tests/doubles/FakeHostElementCatalog.h"

#include <stdexcept>
#include <vector>

namespace cw_api3d::tests::doubles
{

  class FakeHostElementActivation final
  {
  public:
    enum class Behaviour
    {
      Succeeds,
      ReturnsNullptr,
      Throws
    };

    FakeHostElementIdList* createEmptyElementIDList()
    {
      ++mCreateCount;
      switch (mBehaviour)
      {
        case Behaviour::ReturnsNullptr:
          return nullptr;
        case Behaviour::Throws:
          throw std::runtime_error("Simulated host exception");
        case Behaviour::Succeeds:
          break;
      }
      mCreated.setIds({});
      return &mCreated;
    }

    void setActive(FakeHostElementIdList* list)
    {
      ++mSetActiveCount;
      mLastActive = list ? list->ids() : std::vector<application::ElementId>{};
    }

    void setBehaviour(const Behaviour behaviour) noexcept
    {
      mBehaviour = behaviour;
    }

    [[nodiscard]] FakeHostElementIdList& createdList() noexcept
    {
      return mCreated;
    }

    [[nodiscard]] const std::vector<application::ElementId>& lastActive() const noexcept
    {
      return mLastActive;
    }

    [[nodiscard]] int createCount() const noexcept
    {
      return mCreateCount;
    }

    [[nodiscard]] int setActiveCount() const noexcept
    {
      return mSetActiveCount;
    }

  private:
    FakeHostElementIdList mCreated;
    std::vector<application::ElementId> mLastActive;
    Behaviour mBehaviour{Behaviour::Succeeds};
    int mCreateCount{0};
    int mSetActiveCount{0};
  };

} // namespace cw_api3d::tests::doubles
