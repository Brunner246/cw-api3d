#include "src/composition/Bootstrapping.h"
#include "src/adapters/driven/cadwork/CadworkUtilityAdapter.h"
#include "src/adapters/driven/logging/SpdLogLogger.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/doubles/FakeUtilityProvider.h"
#include "tests/doubles/StubCwAPI3DControllerFactory.h"
#include "tests/doubles/StubCwAPI3DUtilityController.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <memory>
#include <optional>

namespace cw_api3d::tests::composition {

using namespace cw_api3d::composition;
using namespace cw_api3d::tests::doubles;
using namespace cw_api3d::ports;

class CompositionRootTests : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(CompositionRootTests, BootstrapperWithFakeAdaptersSuccess) {
    const std::filesystem::path expected_path = "C:/cadwork/plugins/my_plugin";
    auto fake_util = std::make_shared<FakeUtilityProvider>(expected_path);
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);

    PluginBootstrapper bootstrapper(fake_util, fake_logger);
    const auto result = bootstrapper.run();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, expected_path);

    EXPECT_TRUE(fake_logger->has_message(LogLevel::Trace, "Querying plugin path from host utility provider..."));
    EXPECT_TRUE(fake_logger->has_message(LogLevel::Info, "Retrieved plugin path: C:/cadwork/plugins/my_plugin"));
    EXPECT_EQ(bootstrapper.utility_provider(), fake_util);
    EXPECT_EQ(bootstrapper.logger(), fake_logger);
}

TEST_F(CompositionRootTests, BootstrapperWithFakeAdaptersFailureWhenPathUnavailable) {
    auto fake_util = std::make_shared<FakeUtilityProvider>(std::nullopt);
    auto fake_logger = std::make_shared<FakeLogger>(LogLevel::Trace);

    PluginBootstrapper bootstrapper(fake_util, fake_logger);
    const auto result = bootstrapper.run();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Plugin path is not available from utility provider");
    EXPECT_TRUE(fake_logger->has_message(LogLevel::Warn, "Plugin path is not available from utility provider"));
}

TEST_F(CompositionRootTests, BootstrapperHandlesUninitializedDependenciesGracefully) {
    PluginBootstrapper bootstrapper(nullptr, nullptr);
    const auto result = bootstrapper.run();

    ASSERT_FALSE(result.has_value());
}

TEST_F(CompositionRootTests, ProductionBootstrapperWithStubFactorySuccess) {
    StubCwAPI3DUtilityController stub_util("D:/cadwork_plugins/cw_api3d");
    StubCwAPI3DControllerFactory factory(&stub_util);

    auto bootstrapper = PluginBootstrapper::create_production(&factory);
    ASSERT_NE(bootstrapper, nullptr);

    const auto result = bootstrapper->run();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::filesystem::path("D:/cadwork_plugins/cw_api3d"));
}

TEST_F(CompositionRootTests, BootstrapPluginFunctionReturnsTrueOnSuccess) {
    StubCwAPI3DUtilityController stub_util("D:/cadwork_plugins/cw_api3d");
    StubCwAPI3DControllerFactory factory(&stub_util);

    const bool success = bootstrap_plugin(&factory);
    EXPECT_TRUE(success);
}

TEST_F(CompositionRootTests, BootstrapPluginFunctionReturnsFalseOnUnavailablePath) {
    StubCwAPI3DUtilityController stub_util;
    stub_util.set_behaviour(StubCwAPI3DUtilityController::Behaviour::ReturnsNullptr);
    StubCwAPI3DControllerFactory factory(&stub_util);

    const bool success = bootstrap_plugin(&factory);
    EXPECT_FALSE(success);
}

TEST_F(CompositionRootTests, BootstrapPluginHandlesNullFactorySafely) {
    const bool success = bootstrap_plugin(nullptr);
    EXPECT_FALSE(success);
}

TEST_F(CompositionRootTests, BootstrapPluginHandlesHostExceptionsSafely) {
    StubCwAPI3DUtilityController stub_util;
    stub_util.set_behaviour(StubCwAPI3DUtilityController::Behaviour::Throws);
    StubCwAPI3DControllerFactory factory(&stub_util);

    const bool success = bootstrap_plugin(&factory);
    EXPECT_FALSE(success);
}

} // namespace cw_api3d::tests::composition
