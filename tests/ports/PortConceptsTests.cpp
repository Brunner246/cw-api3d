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

TEST(PortConceptsTests, ParseLogLevelCaseInsensitive)
{
  EXPECT_EQ(parseLogLevel("trace"), LogLevel::Trace);
  EXPECT_EQ(parseLogLevel("DEBUG"), LogLevel::Debug);
  EXPECT_EQ(parseLogLevel(" Info "), LogLevel::Info);
  EXPECT_EQ(parseLogLevel("warn"), LogLevel::Warn);
  EXPECT_EQ(parseLogLevel("warning"), LogLevel::Warn);
  EXPECT_EQ(parseLogLevel("error"), LogLevel::Error);
  EXPECT_EQ(parseLogLevel("err"), LogLevel::Error);
  EXPECT_EQ(parseLogLevel("critical"), LogLevel::Critical);
  EXPECT_EQ(parseLogLevel("off"), LogLevel::Off);
  EXPECT_EQ(parseLogLevel("invalid"), std::nullopt);
  EXPECT_EQ(parseLogLevel(""), std::nullopt);
}

TEST(PortConceptsTests, ToStringConversion)
{
  EXPECT_EQ(toString(LogLevel::Trace), "Trace");
  EXPECT_EQ(toString(LogLevel::Debug), "Debug");
  EXPECT_EQ(toString(LogLevel::Info), "Info");
  EXPECT_EQ(toString(LogLevel::Warn), "Warn");
  EXPECT_EQ(toString(LogLevel::Error), "Error");
  EXPECT_EQ(toString(LogLevel::Critical), "Critical");
  EXPECT_EQ(toString(LogLevel::Off), "Off");
}

TEST(PortConceptsTests, FakeLoggerRecordsEntries)
{
  FakeLogger logger(LogLevel::Debug);

  logger.trace("Trace message"); // Should be ignored (below Debug)
  logger.debug("Debug message");
  logger.info("Info message");
  logger.warn("Warn message");
  logger.error("Error message");
  logger.critical("Critical message");

  auto entries = logger.getEntries();
  ASSERT_EQ(entries.size(), 5u);

  EXPECT_FALSE(logger.hasMessage(LogLevel::Trace, "Trace"));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Debug, "Debug message"));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Info, "Info message"));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Warn, "Warn message"));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Error, "Error message"));
  EXPECT_TRUE(logger.hasMessage(LogLevel::Critical, "Critical message"));
}

TEST(PortConceptsTests, FakeLoggerFormattingHelpers)
{
  FakeLogger logger(LogLevel::Info);

  logger.infof("User {} performed action {}", 42, "login");
  EXPECT_TRUE(logger.hasMessage(LogLevel::Info, "User 42 performed action login"));

  logger.clear();
  EXPECT_EQ(logger.count(), 0u);
}

TEST(PortConceptsTests, FakeUtilityProviderReturnsConfiguredPath)
{
  FakeUtilityProvider provider;
  EXPECT_EQ(provider.getPluginPath(), std::nullopt);

  const std::filesystem::path testPath = "C:/Program Files/Cadwork/Plugins/MyPlugin";
  provider.setPluginPath(testPath);

  auto result = provider.getPluginPath();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, testPath);
}

TEST(PortConceptsTests, DynamicPolymorphismViaInterfaces)
{
  std::unique_ptr<interfaces::ILogger> logger = std::make_unique<FakeLogger>();
  logger->info("Dynamic dispatch log");
  auto* fake = dynamic_cast<FakeLogger*>(logger.get());
  ASSERT_NE(fake, nullptr);
  EXPECT_TRUE(fake->hasMessage(LogLevel::Info, "Dynamic dispatch log"));

  std::unique_ptr<interfaces::IUtilityProvider> provider = std::make_unique<FakeUtilityProvider>(std::filesystem::path("D:/cadwork/plugins"));
  EXPECT_EQ(provider->getPluginPath(), std::filesystem::path("D:/cadwork/plugins"));
}
