#include <gtest/gtest.h>

#include "src/application/QueryPluginPathUseCase.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/doubles/FakeUtilityProvider.h"

#include <concepts>
#include <filesystem>
#include <memory>

using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;

// Compile-time checks
static_assert(std::derived_from<QueryPluginPathUseCase, IQueryPluginPathUseCase>,
    "QueryPluginPathUseCase must derive from IQueryPluginPathUseCase");

TEST(QueryPluginPathUseCaseTests, ExecuteStaticSuccessLogsTraceAndInfo) {
    FakeUtilityProvider utility(std::filesystem::path("C:/Cadwork/Plugins/MyPlugin"));
    FakeLogger logger(LogLevel::Trace);

    auto result = QueryPluginPathUseCase::execute_static(utility, logger);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::filesystem::path("C:/Cadwork/Plugins/MyPlugin"));

    EXPECT_TRUE(logger.has_message(LogLevel::Trace, "Querying plugin path"));
    EXPECT_TRUE(logger.has_message(LogLevel::Info, "Retrieved plugin path: C:/Cadwork/Plugins/MyPlugin"));
    EXPECT_FALSE(logger.has_message(LogLevel::Warn, "not available"));
}

TEST(QueryPluginPathUseCaseTests, ExecuteStaticFailureWhenPathUnavailable) {
    FakeUtilityProvider utility(std::nullopt);
    FakeLogger logger(LogLevel::Trace);

    auto result = QueryPluginPathUseCase::execute_static(utility, logger);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Plugin path is not available from utility provider");

    EXPECT_TRUE(logger.has_message(LogLevel::Trace, "Querying plugin path"));
    EXPECT_TRUE(logger.has_message(LogLevel::Warn, "Plugin path is not available"));
}

TEST(QueryPluginPathUseCaseTests, ExecuteDynamicViaInterfaceReferences) {
    FakeUtilityProvider utility(std::filesystem::path("D:/Plugins/Hexagonal"));
    FakeLogger logger(LogLevel::Trace);

    QueryPluginPathUseCase use_case(utility, logger);
    auto result = use_case.execute();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::filesystem::path("D:/Plugins/Hexagonal"));
    EXPECT_TRUE(logger.has_message(LogLevel::Info, "D:/Plugins/Hexagonal"));
}

TEST(QueryPluginPathUseCaseTests, ExecuteDynamicViaSharedPointers) {
    auto utility = std::make_shared<FakeUtilityProvider>(std::filesystem::path("E:/Cadwork/Active"));
    auto logger = std::make_shared<FakeLogger>(LogLevel::Trace);

    std::unique_ptr<IQueryPluginPathUseCase> use_case =
        std::make_unique<QueryPluginPathUseCase>(utility, logger);

    auto result = use_case->execute();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::filesystem::path("E:/Cadwork/Active"));
    EXPECT_TRUE(logger->has_message(LogLevel::Info, "E:/Cadwork/Active"));
}

TEST(QueryPluginPathUseCaseTests, ExecuteDynamicUninitializedHandlesErrorGracefully) {
    QueryPluginPathUseCase use_case(nullptr, nullptr);
    auto result = use_case.execute();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Utility provider or logger dependency is uninitialized");
}
