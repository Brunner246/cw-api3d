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

class SpdLogLoggerTests : public ::testing::Test {
protected:
    void SetUp() override {
        oss_ = std::make_shared<std::ostringstream>();
        auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*oss_);
        raw_spdlog_ = std::make_shared<spdlog::logger>("test_logger", ostream_sink);
        raw_spdlog_->set_pattern("%v"); // Output message only for deterministic assertions
        raw_spdlog_->set_level(spdlog::level::trace);
        logger_ = std::make_unique<SpdLogLogger>(raw_spdlog_);
    }

    std::shared_ptr<std::ostringstream> oss_;
    std::shared_ptr<spdlog::logger> raw_spdlog_;
    std::unique_ptr<SpdLogLogger> logger_;
};

TEST_F(SpdLogLoggerTests, LogsAtAllLevelsWhenTraceEnabled) {
    logger_->set_level(LogLevel::Trace);

    logger_->trace("Trace message");
    logger_->debug("Debug message");
    logger_->info("Info message");
    logger_->warn("Warn message");
    logger_->error("Error message");
    logger_->critical("Critical message");

    const std::string output = oss_->str();
    EXPECT_NE(output.find("Trace message"), std::string::npos);
    EXPECT_NE(output.find("Debug message"), std::string::npos);
    EXPECT_NE(output.find("Info message"), std::string::npos);
    EXPECT_NE(output.find("Warn message"), std::string::npos);
    EXPECT_NE(output.find("Error message"), std::string::npos);
    EXPECT_NE(output.find("Critical message"), std::string::npos);
}

TEST_F(SpdLogLoggerTests, FiltersMessagesBelowConfiguredLevel) {
    logger_->set_level(LogLevel::Warn);

    EXPECT_FALSE(logger_->is_enabled(LogLevel::Trace));
    EXPECT_FALSE(logger_->is_enabled(LogLevel::Debug));
    EXPECT_FALSE(logger_->is_enabled(LogLevel::Info));
    EXPECT_TRUE(logger_->is_enabled(LogLevel::Warn));
    EXPECT_TRUE(logger_->is_enabled(LogLevel::Error));
    EXPECT_TRUE(logger_->is_enabled(LogLevel::Critical));

    logger_->info("Should be dropped");
    logger_->warn("Should be logged");
    logger_->error("Should also be logged");

    const std::string output = oss_->str();
    EXPECT_EQ(output.find("Should be dropped"), std::string::npos);
    EXPECT_NE(output.find("Should be logged"), std::string::npos);
    EXPECT_NE(output.find("Should also be logged"), std::string::npos);
}

TEST_F(SpdLogLoggerTests, FormattingHelpersFormatCorrectly) {
    logger_->set_level(LogLevel::Info);

    logger_->infof("Found {} elements in model {}", 150, "RoofStructure");

    const std::string output = oss_->str();
    EXPECT_NE(output.find("Found 150 elements in model RoofStructure"), std::string::npos);
}

TEST_F(SpdLogLoggerTests, OffLevelDisablesAllLogging) {
    logger_->set_level(LogLevel::Off);
    EXPECT_EQ(logger_->get_level(), LogLevel::Off);
    EXPECT_FALSE(logger_->is_enabled(LogLevel::Critical));

    logger_->critical("Will not be logged");
    EXPECT_TRUE(oss_->str().empty());
}

TEST(SpdLogLoggerStandaloneTests, DefaultConstructorCreatesValidLogger) {
    SpdLogLogger default_logger;
    EXPECT_NE(default_logger.underlying(), nullptr);
    // Should not throw or crash
    default_logger.info("Default logger operational");
}
