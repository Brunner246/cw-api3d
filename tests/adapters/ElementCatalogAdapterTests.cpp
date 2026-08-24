#include <gtest/gtest.h>

#include "src/adapters/driven/cadwork/ElementCatalogAdapter.h"
#include "src/ports/ElementCatalog.h"
#include "tests/doubles/FakeHostElementCatalog.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <cmath>
#include <concepts>
#include <limits>
#include <memory>

using namespace cw_api3d::adapters::driven::cadwork;
using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;
using namespace cw_api3d::tests::fixtures;

using FakeAdapter = ElementCatalogAdapter<FakeHostElementCatalog>;

static_assert(cw_api3d::adapters::driven::cadwork::concepts::ElementCatalogSource<FakeHostElementCatalog>,
              "FakeHostElementCatalog must satisfy concepts::ElementCatalogSource");
static_assert(cw_api3d::ports::concepts::ElementCatalog<FakeAdapter>,
              "ElementCatalogAdapter must satisfy ports::concepts::ElementCatalog");
static_assert(std::derived_from<FakeAdapter, interfaces::IElementCatalog>,
              "ElementCatalogAdapter must derive from IElementCatalog");

TEST(ElementCatalogAdapterTests, CopiesActiveIdsAndNeverDestroysHostObjects)
{
  FakeHostElementCatalog host;
  host.setActive(activeSnapshot());
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&host, fakeLogger);

  const auto result = adapter.fetch(ElementUniverse::Active);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->records.size(), static_cast<std::size_t>(lastActiveId));
  EXPECT_EQ(result->records.front().id, 1u);
  EXPECT_EQ(result->records.back().id, lastActiveId);
  EXPECT_EQ(result->records.front().kind, ElementKind::RectangularBeam);
  EXPECT_EQ(result->records.front().material, "C24");
  EXPECT_FALSE(host.activeList().destroyCalled());
  EXPECT_FALSE(host.nameString().destroyCalled());
  EXPECT_FALSE(host.typeDouble().destroyCalled());
}

TEST(ElementCatalogAdapterTests, MapsSpecificWallBeforeGenericWall)
{
  FakeHostElementCatalog host;
  ElementSnapshot snapshot;
  snapshot.records.push_back(ElementRecord{
    .id = 1,
    .kind = ElementKind::FramedWall,
    .kindLabel = "Framed wall"});
  host.setAll(snapshot);
  const FakeAdapter adapter(&host);

  const auto result = adapter.fetch(ElementUniverse::All);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->records.size(), 1u);
  EXPECT_EQ(result->records.front().kind, ElementKind::FramedWall);
}

TEST(ElementCatalogAdapterTests, MapsRectangularBeamBeforeGenericBeam)
{
  FakeHostElementCatalog host;
  ElementSnapshot snapshot;
  snapshot.records.push_back(recordForId(1));
  host.setAll(snapshot);
  const FakeAdapter adapter(&host);

  const auto result = adapter.fetch(ElementUniverse::All);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->records.front().kind, ElementKind::RectangularBeam);
}

TEST(ElementCatalogAdapterTests, NonFiniteGeometryBecomesNullopt)
{
  FakeHostElementCatalog host;
  ElementSnapshot snapshot;
  snapshot.records.push_back(recordForId(40));
  host.setAll(snapshot);
  const FakeAdapter adapter(&host);

  const auto result = adapter.fetch(ElementUniverse::All);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->records.size(), 1u);
  EXPECT_FALSE(result->records.front().length.has_value());
}

TEST(ElementCatalogAdapterTests, NullHostIsUnexpected)
{
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(nullptr, fakeLogger);

  const auto result = adapter.fetch(ElementUniverse::Active);

  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "host catalog is null"));
}

TEST(ElementCatalogAdapterTests, NullIdListIsUnexpected)
{
  FakeHostElementCatalog host;
  host.setBehaviour(FakeHostElementCatalog::Behaviour::ReturnsNullptr);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&host, fakeLogger);

  const auto result = adapter.fetch(ElementUniverse::All);

  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "null ID list"));
}

TEST(ElementCatalogAdapterTests, HostExceptionIsUnexpected)
{
  FakeHostElementCatalog host;
  host.setBehaviour(FakeHostElementCatalog::Behaviour::Throws);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&host, fakeLogger);

  const auto result = adapter.fetch(ElementUniverse::Active);

  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Error, "Exception fetching snapshot"));
}
