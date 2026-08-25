#include "src/composition/ChartsBootstrap.h"
#include "src/composition/PluginUiSession.h"
#include "tests/doubles/FakeStatisticsPanel.h"

#include <gtest/gtest.h>

namespace cw_api3d::tests::composition
{

  using namespace cw_api3d::composition;
  using namespace cw_api3d::tests::doubles;

  class PluginUiSessionTests : public ::testing::Test
  {
  protected:
    void TearDown() override
    {
      PluginUiSession::resetInstance();
    }
  };

  TEST_F(PluginUiSessionTests, SecondShowOrFocusDoesNotCreateAnotherPanel)
  {
    FakeStatisticsPanel panel;
    PluginUiSession session(panel);

    session.showOrFocus();
    session.showOrFocus();

    EXPECT_EQ(panel.createCount, 1);
    EXPECT_EQ(panel.showCount, 2);
  }

  TEST_F(PluginUiSessionTests, BootstrapChartsPluginCatchesSessionThrowAndReturnsFalse)
  {
    FakeStatisticsPanel panel;
    panel.throwOnShow = true;
    PluginUiSession::resetForTest(panel);

    const bool success = bootstrapChartsPlugin(nullptr);

    EXPECT_FALSE(success);
  }

} // namespace cw_api3d::tests::composition
