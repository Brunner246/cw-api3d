#include <gtest/gtest.h>

#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <set>
#include <string>

using namespace cw_api3d::tests::fixtures;

namespace
{

  [[nodiscard]] std::set<ElementId> idsOf(const ElementSnapshot& snapshot)
  {
    std::set<ElementId> ids;
    for (const auto& record : snapshot.records)
    {
      ids.insert(record.id);
    }
    return ids;
  }

}

TEST(ElementSnapshotFixturesTests, ContainsMixedElementKinds)
{
  const auto snapshot = allSnapshot();
  std::set<ElementKind> kinds;
  for (const auto& record : snapshot.records)
  {
    kinds.insert(record.kind);
  }

  const std::set<ElementKind> expected{
    ElementKind::RectangularBeam,
    ElementKind::CircularBeam,
    ElementKind::Panel,
    ElementKind::Opening,
    ElementKind::Other};
  EXPECT_EQ(kinds, expected);
}

TEST(ElementSnapshotFixturesTests, IncludesEmptyNameAndEmptyMaterialRows)
{
  const auto snapshot = allSnapshot();
  const bool hasEmptyName = std::ranges::any_of(snapshot.records, [](const ElementRecord& record) {
    return record.name.empty();
  });
  const bool hasEmptyMaterial = std::ranges::any_of(snapshot.records, [](const ElementRecord& record) {
    return record.material.empty();
  });

  EXPECT_TRUE(hasEmptyName && hasEmptyMaterial);
}

TEST(ElementSnapshotFixturesTests, AllSnapshotHasFifteenUniqueNonEmptyNames)
{
  const auto snapshot = allSnapshot();
  std::set<std::string> names;
  for (const auto& record : snapshot.records)
  {
    if (!record.name.empty())
    {
      names.insert(record.name);
    }
  }

  EXPECT_EQ(names.size(), uniqueNameCount);
}

TEST(ElementSnapshotFixturesTests, DimensionSpansIncludeFiniteValuesAndNaN)
{
  const auto snapshot = allSnapshot();
  bool hasFinite = false;
  bool hasNaN = false;
  bool hasMissing = false;
  for (const auto& record : snapshot.records)
  {
    if (!record.length.has_value())
    {
      hasMissing = true;
      continue;
    }
    if (std::isnan(*record.length))
    {
      hasNaN = true;
    }
    else if (std::isfinite(*record.length))
    {
      hasFinite = true;
    }
  }

  EXPECT_TRUE(hasFinite && hasNaN && hasMissing);
}

TEST(ElementSnapshotFixturesTests, C24MaterialHasAtLeastThirtyTwoMembers)
{
  const auto snapshot = allSnapshot();
  const auto count = std::ranges::count_if(snapshot.records, [](const ElementRecord& record) {
    return record.material == "C24";
  });

  EXPECT_GE(count, static_cast<std::ptrdiff_t>(c24MemberCount));
}

TEST(ElementSnapshotFixturesTests, ActiveAndAllIdSetsAreDistinct)
{
  EXPECT_NE(idsOf(activeSnapshot()), idsOf(allSnapshot()));
}

TEST(ElementSnapshotFixturesTests, AllIdSetIsProperSupersetOfActive)
{
  const auto activeIds = idsOf(activeSnapshot());
  const auto allIds = idsOf(allSnapshot());

  EXPECT_TRUE(std::ranges::includes(allIds, activeIds) && allIds.size() > activeIds.size());
}
