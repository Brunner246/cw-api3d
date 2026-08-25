#include <gtest/gtest.h>

#include "src/adapters/driven/logging/SpdLogLogger.h"
#include "src/ports/Logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>

#include <concepts>
#include <memory>
#include <sstream>

using namespace cw_api3d::adapters::driven::logging;
using namespace cw_api3d::ports;

// Compile-time static assertions for concepts and interface inheritance
static_assert(concepts::Logger<SpdLogLogger>, "SpdLogLogger must satisfy concepts::Logger");
static_assert(std::derived_from<SpdLogLogger, interfaces::ILogger>, "SpdLogLogger must derive from ILogger");

class SpdLogLoggerTests : public ::testing::Test
{
protected:
  void SetUp() override
  {
    mOss = std::make_shared<std::ostringstream>();
    auto ostreamSink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*mOss);
    mRawSpdlog = std::make_shared<spdlog::logger>("test_logger", ostreamSink);
    mRawSpdlog->set_pattern("%v"); // Output message only for deterministic assertions
    mRawSpdlog->set_level(spdlog::level::trace);
    mLogger = std::make_unique<SpdLogLogger>(mRawSpdlog);
  }

  std::shared_ptr<std::ostringstream> mOss;
  std::shared_ptr<spdlog::logger> mRawSpdlog;
  std::unique_ptr<SpdLogLogger> mLogger;
};

TEST_F(SpdLogLoggerTests, LogsAtAllLevelsWhenTraceEnabled)
{
  mLogger->setLevel(LogLevel::Trace);

  mLogger->trace("Trace message");
  mLogger->debug("Debug message");
  mLogger->info("Info message");
  mLogger->warn("Warn message");
  mLogger->error("Error message");
  mLogger->critical("Critical message");

  const std::string output = mOss->str();
  EXPECT_NE(output.find("Trace message"), std::string::npos);
  EXPECT_NE(output.find("Debug message"), std::string::npos);
  EXPECT_NE(output.find("Info message"), std::string::npos);
  EXPECT_NE(output.find("Warn message"), std::string::npos);
  EXPECT_NE(output.find("Error message"), std::string::npos);
  EXPECT_NE(output.find("Critical message"), std::string::npos);
}

TEST_F(SpdLogLoggerTests, FiltersMessagesBelowConfiguredLevel)
{
  mLogger->setLevel(LogLevel::Warn);

  EXPECT_FALSE(mLogger->isEnabled(LogLevel::Trace));
  EXPECT_FALSE(mLogger->isEnabled(LogLevel::Debug));
  EXPECT_FALSE(mLogger->isEnabled(LogLevel::Info));
  EXPECT_TRUE(mLogger->isEnabled(LogLevel::Warn));
  EXPECT_TRUE(mLogger->isEnabled(LogLevel::Error));
  EXPECT_TRUE(mLogger->isEnabled(LogLevel::Critical));

  mLogger->info("Should be dropped");
  mLogger->warn("Should be logged");
  mLogger->error("Should also be logged");

  const std::string output = mOss->str();
  EXPECT_EQ(output.find("Should be dropped"), std::string::npos);
  EXPECT_NE(output.find("Should be logged"), std::string::npos);
  EXPECT_NE(output.find("Should also be logged"), std::string::npos);
}

TEST_F(SpdLogLoggerTests, FormattingHelpersFormatCorrectly)
{
  mLogger->setLevel(LogLevel::Info);

  mLogger->infof("Found {} elements in model {}", 150, "RoofStructure");

  const std::string output = mOss->str();
  EXPECT_NE(output.find("Found 150 elements in model RoofStructure"), std::string::npos);
}

TEST_F(SpdLogLoggerTests, OffLevelDisablesAllLogging)
{
  mLogger->setLevel(LogLevel::Off);
  EXPECT_EQ(mLogger->getLevel(), LogLevel::Off);
  EXPECT_FALSE(mLogger->isEnabled(LogLevel::Critical));

  mLogger->critical("Will not be logged");
  EXPECT_TRUE(mOss->str().empty());
}

TEST(SpdLogLoggerStandaloneTests, DefaultConstructorCreatesValidLogger)
{
  SpdLogLogger defaultLogger;
  EXPECT_NE(defaultLogger.underlying(), nullptr);
  // Should not throw or crash
  defaultLogger.info("Default logger operational");
}
