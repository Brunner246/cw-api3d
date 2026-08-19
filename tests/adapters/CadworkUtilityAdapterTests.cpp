#include <gtest/gtest.h>

#include "src/adapters/driven/cadwork/CadworkUtilityAdapter.h"
#include "src/ports/UtilityProvider.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/doubles/StubCwAPI3DUtilityController.h"

#include <concepts>
#include <filesystem>
#include <memory>

using namespace cw_api3d::adapters::driven::cadwork;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;

// Compile-time static assertions for concepts and interface inheritance
static_assert(concepts::UtilityProvider<CadworkUtilityAdapter>,
              "CadworkUtilityAdapter must satisfy concepts::UtilityProvider");
static_assert(std::derived_from<CadworkUtilityAdapter, interfaces::IUtilityProvider>,
              "CadworkUtilityAdapter must derive from IUtilityProvider");

TEST(CadworkUtilityAdapterTests, ReturnsValidPathWhenHostProvidesString)
{
  StubCwAPI3DUtilityController controller("C:/Program Files/Cadwork/3d/Plugins/HexPlugin");
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const CadworkUtilityAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, std::filesystem::path("C:/Program Files/Cadwork/3d/Plugins/HexPlugin"));
  EXPECT_EQ(controller.callCount(), 1);

  // Architectural invariant: NEVER call destroy() on host-owned strings
  EXPECT_FALSE(controller.stringStub().destroyCalled());
}

TEST(CadworkUtilityAdapterTests, TrimsSurroundingWhitespaceFromHostString)
{
  StubCwAPI3DUtilityController controller("   \t D:/Cadwork/Plugins/MyTool \r\n  ");
  const CadworkUtilityAdapter adapter(&controller);

  const auto result = adapter.getPluginPath();

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, std::filesystem::path("D:/Cadwork/Plugins/MyTool"));
  EXPECT_FALSE(controller.stringStub().destroyCalled());
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenControllerIsNull)
{
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const CadworkUtilityAdapter adapter(nullptr, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "ICwAPI3DUtilityController is null"));
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenGetPluginPathReturnsNullptr)
{
  StubCwAPI3DUtilityController controller;
  controller.setBehaviour(StubCwAPI3DUtilityController::Behaviour::ReturnsNullptr);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const CadworkUtilityAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "returned null pointer"));
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenNarrowDataReturnsNullptr)
{
  StubCwAPI3DUtilityController controller("Valid/Path");
  controller.stringStub().setReturnNullNarrowData(true);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const CadworkUtilityAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "narrowData() returned null pointer"));
  EXPECT_FALSE(controller.stringStub().destroyCalled());
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenStringIsEmptyOrWhitespace)
{
  StubCwAPI3DUtilityController controller("   \t \r\n  ");
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const CadworkUtilityAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "returned empty string"));
  EXPECT_FALSE(controller.stringStub().destroyCalled());
}

TEST(CadworkUtilityAdapterTests, CatchesHostExceptionAndReturnsNulloptSafely)
{
  StubCwAPI3DUtilityController controller;
  controller.setBehaviour(StubCwAPI3DUtilityController::Behaviour::Throws);
  const auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);
  const CadworkUtilityAdapter adapter(&controller, fakeLogger);

  const auto result = adapter.getPluginPath();

  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Error, "Exception querying plugin path"));
}
