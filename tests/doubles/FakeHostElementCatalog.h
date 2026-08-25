#pragma once

#include "src/application/ElementSnapshot.h"
#include "tests/doubles/FakeHostUtilityController.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cw_api3d::tests::doubles
{

  class FakeHostElementIdList final
  {
  public:
    std::uint32_t count() noexcept
    {
      return static_cast<std::uint32_t>(mIds.size());
    }

    application::ElementId at(const std::uint32_t index)
    {
      return mIds.at(index);
    }

    void append(const application::ElementId id)
    {
      mIds.push_back(id);
    }

    void destroy() noexcept
    {
      mDestroyCalled = true;
    }

    void setIds(std::vector<application::ElementId> ids)
    {
      mIds = std::move(ids);
    }

    [[nodiscard]] const std::vector<application::ElementId>& ids() const noexcept
    {
      return mIds;
    }

    [[nodiscard]] bool destroyCalled() const noexcept
    {
      return mDestroyCalled;
    }

  private:
    std::vector<application::ElementId> mIds;
    bool mDestroyCalled{false};
  };

  class FakeHostElementType final
  {
  public:
    void setKind(const application::ElementKind kind) noexcept
    {
      mKind = kind;
      mExtraPanel = false;
      mExtraRectangularBeam = false;
      mExtraOpening = false;
    }

    void addFlag(const application::ElementKind flag) noexcept
    {
      if (flag == application::ElementKind::Panel)
      {
        mExtraPanel = true;
        return;
      }
      if (flag == application::ElementKind::RectangularBeam)
      {
        mExtraRectangularBeam = true;
        return;
      }
      if (flag == application::ElementKind::Opening)
      {
        mExtraOpening = true;
      }
    }

    [[nodiscard]] bool isFramedWall() const noexcept
    {
      return mKind == application::ElementKind::FramedWall;
    }

    [[nodiscard]] bool isSolidWoodWall() const noexcept
    {
      return mKind == application::ElementKind::SolidWoodWall;
    }

    [[nodiscard]] bool isLogWall() const noexcept
    {
      return mKind == application::ElementKind::LogWall;
    }

    [[nodiscard]] bool isWall() const noexcept
    {
      return mKind == application::ElementKind::FramedWall
             || mKind == application::ElementKind::SolidWoodWall
             || mKind == application::ElementKind::LogWall
             || mKind == application::ElementKind::Wall;
    }

    [[nodiscard]] bool isRectangularBeam() const noexcept
    {
      return mKind == application::ElementKind::RectangularBeam || mExtraRectangularBeam;
    }

    [[nodiscard]] bool isCircularBeam() const noexcept
    {
      return mKind == application::ElementKind::CircularBeam;
    }

    [[nodiscard]] bool isPanel() const noexcept
    {
      return mKind == application::ElementKind::Panel || mExtraPanel;
    }

    [[nodiscard]] bool isOpening() const noexcept
    {
      return mKind == application::ElementKind::Opening || mExtraOpening;
    }

    void destroy() noexcept
    {
      mDestroyCalled = true;
    }

    [[nodiscard]] bool destroyCalled() const noexcept
    {
      return mDestroyCalled;
    }

  private:
    application::ElementKind mKind{application::ElementKind::Other};
    bool mExtraPanel{false};
    bool mExtraRectangularBeam{false};
    bool mExtraOpening{false};
    bool mDestroyCalled{false};
  };

  class FakeHostElementCatalog final
  {
  public:
    enum class Behaviour
    {
      ReturnsList,
      ReturnsNullptr,
      Throws
    };

    void setActive(application::ElementSnapshot snapshot)
    {
      mActive = std::move(snapshot);
    }

    void setAll(application::ElementSnapshot snapshot)
    {
      mAll = std::move(snapshot);
    }

    void setBehaviour(const Behaviour behaviour) noexcept
    {
      mBehaviour = behaviour;
    }

    void addOverlappingTypeFlag(const application::ElementKind flag)
    {
      mOverlappingFlags.push_back(flag);
    }

    FakeHostElementIdList* getActiveIdentifiableElementIDs()
    {
      return listFor(mActive, mActiveList);
    }

    FakeHostElementIdList* getAllIdentifiableElementIDs()
    {
      return listFor(mAll, mAllList);
    }

    FakeHostString* getName(const application::ElementId id)
    {
      mName.setNarrow(find(id).name);
      return &mName;
    }

    FakeHostString* getElementMaterialName(const application::ElementId id)
    {
      mMaterial.setNarrow(find(id).material);
      return &mMaterial;
    }

    FakeHostElementType* getElementType(const application::ElementId id)
    {
      mType.setKind(find(id).kind);
      for (const auto flag : mOverlappingFlags)
      {
        mType.addFlag(flag);
      }
      return &mType;
    }

    [[nodiscard]] double getLength(const application::ElementId id) const
    {
      return numeric(find(id).length);
    }

    [[nodiscard]] double getWidth(const application::ElementId id) const
    {
      return numeric(find(id).width);
    }

    [[nodiscard]] double getHeight(const application::ElementId id) const
    {
      return numeric(find(id).height);
    }

    [[nodiscard]] double getVolume(const application::ElementId id) const
    {
      return numeric(find(id).volume);
    }

    [[nodiscard]] FakeHostElementIdList& activeList() noexcept
    {
      return mActiveList;
    }

    [[nodiscard]] FakeHostElementIdList& allList() noexcept
    {
      return mAllList;
    }

    [[nodiscard]] FakeHostString& nameString() noexcept
    {
      return mName;
    }

    [[nodiscard]] FakeHostElementType& typeDouble() noexcept
    {
      return mType;
    }

  private:
    FakeHostElementIdList* listFor(const application::ElementSnapshot& snapshot, FakeHostElementIdList& list)
    {
      ++mCallCount;
      switch (mBehaviour)
      {
        case Behaviour::ReturnsNullptr:
          return nullptr;
        case Behaviour::Throws:
          throw std::runtime_error("Simulated host exception");
        case Behaviour::ReturnsList:
          break;
      }
      std::vector<application::ElementId> ids;
      ids.reserve(snapshot.records.size());
      for (const auto& record : snapshot.records)
      {
        ids.push_back(record.id);
      }
      list.setIds(std::move(ids));
      return &list;
    }

    [[nodiscard]] const application::ElementRecord& find(const application::ElementId id) const
    {
      for (const auto& record : mActive.records)
      {
        if (record.id == id)
        {
          return record;
        }
      }
      for (const auto& record : mAll.records)
      {
        if (record.id == id)
        {
          return record;
        }
      }
      return mMissing;
    }

    [[nodiscard]] static double numeric(const std::optional<double>& value)
    {
      if (!value.has_value())
      {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return *value;
    }

    application::ElementSnapshot mActive;
    application::ElementSnapshot mAll;
    application::ElementRecord mMissing;
    FakeHostElementIdList mActiveList;
    FakeHostElementIdList mAllList;
    FakeHostString mName;
    FakeHostString mMaterial;
    FakeHostElementType mType;
    std::vector<application::ElementKind> mOverlappingFlags;
    Behaviour mBehaviour{Behaviour::ReturnsList};
    int mCallCount{0};
  };

} // namespace cw_api3d::tests::doubles
