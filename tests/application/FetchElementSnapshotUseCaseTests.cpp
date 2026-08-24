#include <gtest/gtest.h>

#include "src/application/FetchElementSnapshotUseCase.h"
#include "tests/doubles/FakeElementCatalog.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <concepts>
#include <memory>
#include <set>

using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;
using namespace cw_api3d::tests::fixtures;

static_assert(std::derived_from<FetchElementSnapshotUseCase, IFetchElementSnapshotUseCase>,
              "FetchElementSnapshotUseCase must derive from IFetchElementSnapshotUseCase");

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

TEST(FetchElementSnapshotUseCaseTests, ExecuteActiveReturnsFixtureIdMultiset)
{
  FakeElementCatalog catalog;
  catalog.setActive(activeSnapshot());
  catalog.setAll(allSnapshot());
  FakeLogger logger(LogLevel::Trace);
  FetchElementSnapshotUseCase useCase(catalog, logger);

  const auto result = useCase.execute(ElementUniverse::Active);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(idsOf(*result), idsOf(activeSnapshot()));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Trace, "Fetching element snapshot"));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Info, "universe Active"));
}

TEST(FetchElementSnapshotUseCaseTests, ExecuteAllReturnsFixtureIdMultiset)
{
  FakeElementCatalog catalog;
  catalog.setActive(activeSnapshot());
  catalog.setAll(allSnapshot());
  FakeLogger logger(LogLevel::Trace);
  FetchElementSnapshotUseCase useCase(catalog, logger);

  const auto result = useCase.execute(ElementUniverse::All);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(idsOf(*result), idsOf(allSnapshot()));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Info, "universe All"));
}

TEST(FetchElementSnapshotUseCaseTests, EmptySnapshotIsSuccessNotError)
{
  FakeElementCatalog catalog;
  FakeLogger logger(LogLevel::Trace);
  FetchElementSnapshotUseCase useCase(catalog, logger);

  const auto result = useCase.execute(ElementUniverse::Active);

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->records.empty());
}

TEST(FetchElementSnapshotUseCaseTests, HostFailureIsUnexpected)
{
  FakeElementCatalog catalog;
  catalog.setFailure("host failed");
  FakeLogger logger(LogLevel::Trace);
  FetchElementSnapshotUseCase useCase(catalog, logger);

  const auto result = useCase.execute(ElementUniverse::All);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "host failed");
  EXPECT_TRUE(logger.hasMessage(LogLevel::Warn, "host failed"));
}

TEST(FetchElementSnapshotUseCaseTests, ExecuteViaSharedPointers)
{
  auto catalog = std::make_shared<FakeElementCatalog>();
  catalog->setAll(allSnapshot());
  auto logger = std::make_shared<FakeLogger>(LogLevel::Trace);
  std::unique_ptr<IFetchElementSnapshotUseCase> useCase =
    std::make_unique<FetchElementSnapshotUseCase>(catalog, logger);

  const auto result = useCase->execute(ElementUniverse::All);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->records.size(), static_cast<std::size_t>(lastAllId));
}

TEST(FetchElementSnapshotUseCaseTests, ExecuteUninitializedHandlesErrorGracefully)
{
  FetchElementSnapshotUseCase useCase(nullptr, nullptr);

  const auto result = useCase.execute(ElementUniverse::Active);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "Catalog or logger dependency is uninitialized");
}
