#include <gtest/gtest.h>

#include "src/adapters/driven/cadwork/UtilityControllerAdapter.h"
#include "src/ports/UtilityProvider.h"
#include "tests/doubles/FakeHostUtilityController.h"
#include "tests/doubles/FakeLogger.h"

#include <concepts>
#include <filesystem>
#include <memory>

using namespace cw_api3d::adapters::driven::cadwork;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;

using FakeAdapter = UtilityControllerAdapter<FakeHostUtilityController>;

// Compile-time assertions for the narrowed host contract and port interface inheritance
static_assert(cw_api3d::adapters::driven::cadwork::concepts::PluginPathSource<FakeHostUtilityController>,
              "FakeHostUtilityController must satisfy concepts::PluginPathSource");
static_assert(std::derived_from<FakeAdapter, IUtilityProvider>,
              "UtilityControllerAdapter must derive from IUtilityProvider");

TEST(UtilityControllerAdapterTests, ReturnsValidPathWhenHostProvidesString)
{
  FakeHostUtilityController controller("C:/Program Files/Cadwork/3d/Plugins/HexPlugin");
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, std::filesystem::path("C:/Program Files/Cadwork/3d/Plugins/HexPlugin"));
  EXPECT_EQ(controller.callCount(), 1);

  // Architectural invariant: NEVER call destroy() on host-owned strings
  EXPECT_FALSE(controller.stringDouble().destroyCalled());
}

TEST(UtilityControllerAdapterTests, TrimsSurroundingWhitespaceFromHostString)
{
  FakeHostUtilityController controller("   \t D:/Cadwork/Plugins/MyTool \r\n  ");
  const FakeAdapter adapter(&controller);

  const auto result = adapter.getPluginPath();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, std::filesystem::path("D:/Cadwork/Plugins/MyTool"));
  EXPECT_FALSE(controller.stringDouble().destroyCalled());
}

TEST(UtilityControllerAdapterTests, ReturnsNulloptWhenControllerIsNull)
{
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(nullptr, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "host utility controller is null"));
}

TEST(UtilityControllerAdapterTests, ReturnsNulloptWhenGetPluginPathReturnsNullptr)
{
  FakeHostUtilityController controller;
  controller.setBehaviour(FakeHostUtilityController::Behaviour::ReturnsNullptr);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "returned null pointer"));
}

TEST(UtilityControllerAdapterTests, ReturnsNulloptWhenNarrowDataReturnsNullptr)
{
  FakeHostUtilityController controller("Valid/Path");
  controller.stringDouble().setReturnNullNarrowData(true);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "narrowData() returned null pointer"));
  EXPECT_FALSE(controller.stringDouble().destroyCalled());
}

TEST(UtilityControllerAdapterTests, ReturnsNulloptWhenStringIsEmptyOrWhitespace)
{
  FakeHostUtilityController controller("   \t \r\n  ");
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "returned empty string"));
  EXPECT_FALSE(controller.stringDouble().destroyCalled());
}

TEST(UtilityControllerAdapterTests, CatchesHostExceptionAndReturnsNulloptSafely)
{
  FakeHostUtilityController controller;
  controller.setBehaviour(FakeHostUtilityController::Behaviour::Throws);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const FakeAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Error, "Exception querying plugin path"));
}
