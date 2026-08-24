#include <gtest/gtest.h>

#include "src/application/ActivateElementsUseCase.h"
#include "src/application/ElementStatisticsAggregator.h"
#include "tests/doubles/FakeElementActivation.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <algorithm>
#include <concepts>
#include <memory>
#include <ranges>
#include <span>
#include <vector>

using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;
using namespace cw_api3d::tests::fixtures;

static_assert(std::derived_from<ActivateElementsUseCase, IActivateElementsUseCase>,
              "ActivateElementsUseCase must derive from IActivateElementsUseCase");

TEST(ActivateElementsUseCaseTests, RecordsGivenIdSpan)
{
  FakeElementActivation activation;
  FakeLogger logger(LogLevel::Trace);
  ActivateElementsUseCase useCase(activation, logger);
  const std::vector<ElementId> ids{7, 11, 13};

  const auto result = useCase.execute(ids);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(activation.lastIds(), ids);
  EXPECT_EQ(activation.callCount(), 1);
  EXPECT_TRUE(logger.hasMessage(LogLevel::Info, "Activating 3 elements"));
}

TEST(ActivateElementsUseCaseTests, EmptySpanIsSuccessNoOp)
{
  FakeElementActivation activation;
  FakeLogger logger(LogLevel::Trace);
  ActivateElementsUseCase useCase(activation, logger);

  const auto result = useCase.execute(std::span<const ElementId>{});

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(activation.callCount(), 0);
  EXPECT_TRUE(activation.lastIds().empty());
  EXPECT_TRUE(logger.hasMessage(LogLevel::Info, "Activating 0 elements"));
}

TEST(ActivateElementsUseCaseTests, LargeC24SetRoundTrips)
{
  FakeElementActivation activation;
  FakeLogger logger(LogLevel::Trace);
  ActivateElementsUseCase useCase(activation, logger);
  const auto series = ElementStatisticsAggregator{}.aggregate(allSnapshot(), StatisticAxis::Material);
  const auto c24 = std::ranges::find_if(series.buckets, [](const StatisticBucket& bucket) {
    return bucket.label == "C24";
  });
  ASSERT_NE(c24, series.buckets.end());

  const auto result = useCase.execute(c24->memberIds);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(activation.lastIds().size(), c24MemberCount);
  EXPECT_EQ(activation.lastIds(), c24->memberIds);
}

TEST(ActivateElementsUseCaseTests, EmptyAttributeIdsRoundTrip)
{
  FakeElementActivation activation;
  FakeLogger logger(LogLevel::Trace);
  ActivateElementsUseCase useCase(activation, logger);
  const std::vector<ElementId> emptyAttributeIds{38, 39};

  const auto result = useCase.execute(emptyAttributeIds);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(activation.lastIds(), emptyAttributeIds);
}

TEST(ActivateElementsUseCaseTests, HostFailureIsUnexpected)
{
  FakeElementActivation activation;
  activation.setFailure("host failed");
  FakeLogger logger(LogLevel::Trace);
  ActivateElementsUseCase useCase(activation, logger);
  const std::vector<ElementId> ids{1};

  const auto result = useCase.execute(ids);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "host failed");
  EXPECT_TRUE(logger.hasMessage(LogLevel::Warn, "host failed"));
}

TEST(ActivateElementsUseCaseTests, ExecuteUninitializedHandlesErrorGracefully)
{
  ActivateElementsUseCase useCase(nullptr, nullptr);

  const auto result = useCase.execute(std::span<const ElementId>{});

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "Activation or logger dependency is uninitialized");
}
