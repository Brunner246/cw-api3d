#include <gtest/gtest.h>

#include "src/ports/Logger.h"
#include "src/ports/UtilityProvider.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/doubles/FakeUtilityProvider.h"

using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;

// Compile-time static assertions for C++20 concepts
static_assert(concepts::Logger<FakeLogger>, "FakeLogger must satisfy concepts::Logger");
static_assert(concepts::UtilityProvider<FakeUtilityProvider>, "FakeUtilityProvider must satisfy concepts::UtilityProvider");
static_assert(std::derived_from<FakeLogger, interfaces::ILogger>, "FakeLogger must derive from ILogger");
static_assert(std::derived_from<FakeUtilityProvider, interfaces::IUtilityProvider>, "FakeUtilityProvider must derive from IUtilityProvider");

TEST(PortConceptsTests, ParseLogLevelCaseInsensitive) {
    EXPECT_EQ(parse_log_level("trace"), LogLevel::Trace);
    EXPECT_EQ(parse_log_level("DEBUG"), LogLevel::Debug);
    EXPECT_EQ(parse_log_level(" Info "), LogLevel::Info);
    EXPECT_EQ(parse_log_level("warn"), LogLevel::Warn);
    EXPECT_EQ(parse_log_level("warning"), LogLevel::Warn);
    EXPECT_EQ(parse_log_level("error"), LogLevel::Error);
    EXPECT_EQ(parse_log_level("err"), LogLevel::Error);
    EXPECT_EQ(parse_log_level("critical"), LogLevel::Critical);
    EXPECT_EQ(parse_log_level("off"), LogLevel::Off);
    EXPECT_EQ(parse_log_level("invalid"), std::nullopt);
    EXPECT_EQ(parse_log_level(""), std::nullopt);
}

TEST(PortConceptsTests, ToStringConversion) {
    EXPECT_EQ(to_string(LogLevel::Trace), "Trace");
    EXPECT_EQ(to_string(LogLevel::Debug), "Debug");
    EXPECT_EQ(to_string(LogLevel::Info), "Info");
    EXPECT_EQ(to_string(LogLevel::Warn), "Warn");
    EXPECT_EQ(to_string(LogLevel::Error), "Error");
    EXPECT_EQ(to_string(LogLevel::Critical), "Critical");
    EXPECT_EQ(to_string(LogLevel::Off), "Off");
}

TEST(PortConceptsTests, FakeLoggerRecordsEntries) {
    FakeLogger logger(LogLevel::Debug);

    logger.trace("Trace message"); // Should be ignored (below Debug)
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warn("Warn message");
    logger.error("Error message");
    logger.critical("Critical message");

    auto entries = logger.get_entries();
    ASSERT_EQ(entries.size(), 5u);

    EXPECT_FALSE(logger.has_message(LogLevel::Trace, "Trace"));
    EXPECT_TRUE(logger.has_message(LogLevel::Debug, "Debug message"));
    EXPECT_TRUE(logger.has_message(LogLevel::Info, "Info message"));
    EXPECT_TRUE(logger.has_message(LogLevel::Warn, "Warn message"));
    EXPECT_TRUE(logger.has_message(LogLevel::Error, "Error message"));
    EXPECT_TRUE(logger.has_message(LogLevel::Critical, "Critical message"));
}

TEST(PortConceptsTests, FakeLoggerFormattingHelpers) {
    FakeLogger logger(LogLevel::Info);

    logger.infof("User {} performed action {}", 42, "login");
    EXPECT_TRUE(logger.has_message(LogLevel::Info, "User 42 performed action login"));

    logger.clear();
    EXPECT_EQ(logger.count(), 0u);
}

TEST(PortConceptsTests, FakeUtilityProviderReturnsConfiguredPath) {
    FakeUtilityProvider provider;
    EXPECT_EQ(provider.get_plugin_path(), std::nullopt);

    const std::filesystem::path test_path = "C:/Program Files/Cadwork/Plugins/MyPlugin";
    provider.set_plugin_path(test_path);

    auto result = provider.get_plugin_path();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, test_path);
}

TEST(PortConceptsTests, DynamicPolymorphismViaInterfaces) {
    std::unique_ptr<interfaces::ILogger> logger = std::make_unique<FakeLogger>();
    logger->info("Dynamic dispatch log");
    auto* fake = dynamic_cast<FakeLogger*>(logger.get());
    ASSERT_NE(fake, nullptr);
    EXPECT_TRUE(fake->has_message(LogLevel::Info, "Dynamic dispatch log"));

    std::unique_ptr<interfaces::IUtilityProvider> provider = 
        std::make_unique<FakeUtilityProvider>(std::filesystem::path("D:/cadwork/plugins"));
    EXPECT_EQ(provider->get_plugin_path(), std::filesystem::path("D:/cadwork/plugins"));
}
