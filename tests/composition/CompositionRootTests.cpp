#include "src/composition/Bootstrapping.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/doubles/FakeUtilityProvider.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <memory>
#include <optional>

namespace cw_api3d::tests::composition
{

  using namespace cw_api3d::composition;
  using namespace cw_api3d::tests::doubles;
  using namespace cw_api3d::ports;

  TEST(CompositionRootTests, BootstrapperWithFakeAdaptersSuccess)
  {
    const std::filesystem::path expectedPath = "C:/cadwork/plugins/my_plugin";
    auto fakeUtil = std::make_shared<FakeUtilityProvider>(expectedPath);
    auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);

    PluginBootstrapper bootstrapper(fakeUtil, fakeLogger);
    const auto result = bootstrapper.run();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, expectedPath);

    EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Trace, "Querying plugin path from host utility provider..."));
    EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Info, "Retrieved plugin path: C:/cadwork/plugins/my_plugin"));
    EXPECT_EQ(bootstrapper.utilityProvider(), fakeUtil);
    EXPECT_EQ(bootstrapper.logger(), fakeLogger);
  }

  TEST(CompositionRootTests, BootstrapperWithFakeAdaptersFailureWhenPathUnavailable)
  {
    auto fakeUtil = std::make_shared<FakeUtilityProvider>(std::nullopt);
    auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);

    PluginBootstrapper bootstrapper(fakeUtil, fakeLogger);
    const auto result = bootstrapper.run();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Plugin path is not available from utility provider");
    EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "Plugin path is not available from utility provider"));
  }

  TEST(CompositionRootTests, BootstrapperHandlesUninitializedDependenciesGracefully)
  {
    PluginBootstrapper bootstrapper(nullptr, nullptr);
    const auto result = bootstrapper.run();

    ASSERT_FALSE(result.has_value());
  }

  TEST(CompositionRootTests, ProductionBootstrapperResolvesPathFromProvider)
  {
    const std::filesystem::path expectedPath = "D:/cadwork_plugins/cw_api3d";
    const auto bootstrapper = PluginBootstrapper::createProductionForUtilityProvider(
      std::make_shared<FakeUtilityProvider>(expectedPath));
    ASSERT_NE(bootstrapper, nullptr);

    const auto result = bootstrapper->run();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, expectedPath);
  }

  TEST(CompositionRootTests, ProductionBootstrapperFailsWhenProviderHasNoPath)
  {
    const auto bootstrapper = PluginBootstrapper::createProductionForUtilityProvider(
      std::make_shared<FakeUtilityProvider>(std::nullopt));
    ASSERT_NE(bootstrapper, nullptr);

    const auto result = bootstrapper->run();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Plugin path is not available from utility provider");
  }

  TEST(CompositionRootTests, ProductionBootstrapperConfiguresExpectedLogLevel)
  {
    const auto bootstrapper = PluginBootstrapper::createProductionForUtilityProvider(
      std::make_shared<FakeUtilityProvider>("D:/cadwork_plugins/cw_api3d"));
    ASSERT_NE(bootstrapper, nullptr);
    ASSERT_NE(bootstrapper->logger(), nullptr);

#if defined(CW_BUILD_RELWITHDEBINFO)
    EXPECT_EQ(bootstrapper->logger()->getLevel(), LogLevel::Debug);
#elif defined(CW_BUILD_RELEASE)
    EXPECT_EQ(bootstrapper->logger()->getLevel(), LogLevel::Info);
#elif defined(CW_BUILD_DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    EXPECT_EQ(bootstrapper->logger()->getLevel(), LogLevel::Trace);
#else
    EXPECT_EQ(bootstrapper->logger()->getLevel(), LogLevel::Info);
#endif
  }

  // The host factory path cannot succeed without a real cadwork controller; its success wiring is
  // covered by the e2e harness (tests/e2e/test_plugin_e2e.py), which loads the plugin in cadwork.
  TEST(CompositionRootTests, BootstrapPluginHandlesNullFactorySafely)
  {
    const bool success = bootstrapPlugin(nullptr);
    EXPECT_FALSE(success);
  }

} // namespace cw_api3d::tests::composition
