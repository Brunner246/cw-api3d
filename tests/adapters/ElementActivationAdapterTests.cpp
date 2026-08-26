#include <gtest/gtest.h>

#include "src/adapters/driven/cadwork/ElementActivationAdapter.h"
#include "src/ports/ElementActivation.h"
#include "tests/doubles/FakeHostElementActivation.h"
#include "tests/doubles/FakeLogger.h"

#include <concepts>
#include <memory>
#include <vector>

using namespace cw_api3d::adapters::driven::cadwork;
using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;

using FakeAdapter = ElementActivationAdapter<FakeHostElementActivation>;

static_assert(cw_api3d::adapters::driven::cadwork::concepts::ElementActivationSource<FakeHostElementActivation>,
              "FakeHostElementActivation must satisfy concepts::ElementActivationSource");
static_assert(std::derived_from<FakeAdapter, IElementActivation>,
              "ElementActivationAdapter must derive from IElementActivation");

TEST(ElementActivationAdapterTests, AppendsIdsAndNeverDestroysList)
{
  FakeHostElementActivation host;
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  FakeAdapter adapter(&host, fakeLogger);
  const std::vector<ElementId> ids{1, 2, 32};

  const auto result = adapter.activate(ids);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(host.lastActive(), ids);
  EXPECT_EQ(host.createCount(), 1);
  EXPECT_EQ(host.setActiveCount(), 1);
  EXPECT_FALSE(host.createdList().destroyCalled());
}

TEST(ElementActivationAdapterTests, EmptySpanDoesNotCreateList)
{
  FakeHostElementActivation host;
  FakeAdapter adapter(&host);

  const auto result = adapter.activate({});

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(host.createCount(), 0);
  EXPECT_EQ(host.setActiveCount(), 0);
}

TEST(ElementActivationAdapterTests, NullHostIsUnexpected)
{
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  FakeAdapter adapter(nullptr, fakeLogger);
  const std::vector<ElementId> ids{1};

  const auto result = adapter.activate(ids);

  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "host activation is null"));
}

TEST(ElementActivationAdapterTests, NullCreatedListIsUnexpected)
{
  FakeHostElementActivation host;
  host.setBehaviour(FakeHostElementActivation::Behaviour::ReturnsNullptr);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  FakeAdapter adapter(&host, fakeLogger);
  const std::vector<ElementId> ids{1};

  const auto result = adapter.activate(ids);

  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "null ID list"));
}

TEST(ElementActivationAdapterTests, HostExceptionIsUnexpected)
{
  FakeHostElementActivation host;
  host.setBehaviour(FakeHostElementActivation::Behaviour::Throws);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  FakeAdapter adapter(&host, fakeLogger);
  const std::vector<ElementId> ids{1};

  const auto result = adapter.activate(ids);

  ASSERT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Error, "Exception activating elements"));
}
