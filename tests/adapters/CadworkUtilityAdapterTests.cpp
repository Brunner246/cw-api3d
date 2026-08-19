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

TEST(CadworkUtilityAdapterTests, ReturnsValidPathWhenHostProvidesString) {
    StubCwAPI3DUtilityController controller("C:/Program Files/Cadwork/3d/Plugins/HexPlugin");
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);
    CadworkUtilityAdapter adapter(&controller, fake_logger);

    auto result = adapter.get_plugin_path();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::filesystem::path("C:/Program Files/Cadwork/3d/Plugins/HexPlugin"));
    EXPECT_EQ(controller.call_count(), 1);

    // Architectural invariant: NEVER call destroy() on host-owned strings
    EXPECT_FALSE(controller.string_stub().destroy_called());
}

TEST(CadworkUtilityAdapterTests, TrimsSurroundingWhitespaceFromHostString) {
    StubCwAPI3DUtilityController controller("   \t D:/Cadwork/Plugins/MyTool \r\n  ");
    CadworkUtilityAdapter adapter(&controller);

    auto result = adapter.get_plugin_path();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::filesystem::path("D:/Cadwork/Plugins/MyTool"));
    EXPECT_FALSE(controller.string_stub().destroy_called());
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenControllerIsNull) {
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);
    CadworkUtilityAdapter adapter(nullptr, fake_logger);

    auto result = adapter.get_plugin_path();

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(fake_logger->has_message(LogLevel::Warn, "ICwAPI3DUtilityController is null"));
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenGetPluginPathReturnsNullptr) {
    StubCwAPI3DUtilityController controller;
    controller.set_behaviour(StubCwAPI3DUtilityController::Behaviour::ReturnsNullptr);
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);
    CadworkUtilityAdapter adapter(&controller, fake_logger);

    auto result = adapter.get_plugin_path();

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(fake_logger->has_message(LogLevel::Warn, "returned null pointer"));
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenNarrowDataReturnsNullptr) {
    StubCwAPI3DUtilityController controller("Valid/Path");
    controller.string_stub().set_return_null_narrow_data(true);
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);
    CadworkUtilityAdapter adapter(&controller, fake_logger);

    auto result = adapter.get_plugin_path();

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(fake_logger->has_message(LogLevel::Warn, "narrowData() returned null pointer"));
    EXPECT_FALSE(controller.string_stub().destroy_called());
}

TEST(CadworkUtilityAdapterTests, ReturnsNulloptWhenStringIsEmptyOrWhitespace) {
    StubCwAPI3DUtilityController controller("   \t \r\n  ");
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);
    CadworkUtilityAdapter adapter(&controller, fake_logger);

    auto result = adapter.get_plugin_path();

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(fake_logger->has_message(LogLevel::Warn, "returned empty string"));
    EXPECT_FALSE(controller.string_stub().destroy_called());
}

TEST(CadworkUtilityAdapterTests, CatchesHostExceptionAndReturnsNulloptSafely) {
    StubCwAPI3DUtilityController controller;
    controller.set_behaviour(StubCwAPI3DUtilityController::Behaviour::Throws);
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);
    CadworkUtilityAdapter adapter(&controller, fake_logger);

    auto result = adapter.get_plugin_path();

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(fake_logger->has_message(LogLevel::Error, "Exception querying plugin path"));
}
