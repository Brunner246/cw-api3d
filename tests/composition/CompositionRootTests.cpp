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

TEST_F(CompositionRootTests, BootstrapperWithFakeAdaptersFailureWhenPathUnavailable) {
    auto fakeUtil = std::make_shared<FakeUtilityProvider>(std::nullopt);
    auto fakeLogger = std::make_shared<FakeLogger>(LogLevel::Trace);

    PluginBootstrapper bootstrapper(fakeUtil, fakeLogger);
    const auto result = bootstrapper.run();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Plugin path is not available from utility provider");
    EXPECT_TRUE(fakeLogger->hasMessage(LogLevel::Warn, "Plugin path is not available from utility provider"));
}

TEST_F(CompositionRootTests, BootstrapperHandlesUninitializedDependenciesGracefully) {
    PluginBootstrapper bootstrapper(nullptr, nullptr);
    const auto result = bootstrapper.run();

    ASSERT_FALSE(result.has_value());
}

TEST_F(CompositionRootTests, ProductionBootstrapperWithStubFactorySuccess) {
    StubCwAPI3DUtilityController stubUtil("D:/cadwork_plugins/cw_api3d");
    StubCwAPI3DControllerFactory factory(&stubUtil);

    auto bootstrapper = PluginBootstrapper::createProduction(&factory);
    ASSERT_NE(bootstrapper, nullptr);

    const auto result = bootstrapper->run();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, std::filesystem::path("D:/cadwork_plugins/cw_api3d"));
}

TEST_F(CompositionRootTests, BootstrapPluginFunctionReturnsTrueOnSuccess) {
    StubCwAPI3DUtilityController stubUtil("D:/cadwork_plugins/cw_api3d");
    StubCwAPI3DControllerFactory factory(&stubUtil);

    const bool success = bootstrapPlugin(&factory);
    EXPECT_TRUE(success);
}

TEST_F(CompositionRootTests, BootstrapPluginFunctionReturnsFalseOnUnavailablePath) {
    StubCwAPI3DUtilityController stubUtil;
    stubUtil.setBehaviour(StubCwAPI3DUtilityController::Behaviour::ReturnsNullptr);
    StubCwAPI3DControllerFactory factory(&stubUtil);

    const bool success = bootstrapPlugin(&factory);
    EXPECT_FALSE(success);
}

TEST_F(CompositionRootTests, BootstrapPluginHandlesNullFactorySafely) {
    const bool success = bootstrapPlugin(nullptr);
    EXPECT_FALSE(success);
}

TEST_F(CompositionRootTests, BootstrapPluginHandlesHostExceptionsSafely) {
    StubCwAPI3DUtilityController stubUtil;
    stubUtil.setBehaviour(StubCwAPI3DUtilityController::Behaviour::Throws);
    StubCwAPI3DControllerFactory factory(&stubUtil);

    const bool success = bootstrapPlugin(&factory);
    EXPECT_FALSE(success);
}

} // namespace cw_api3d::tests::composition
