#include <gtest/gtest.h>

#include "tests/fixtures/ContrastTokenPairs.h"

#include <ranges>

using namespace cw_api3d::tests::fixtures;

TEST(ContrastTokenPairsTests, RelativeLuminanceOfBlackIsZero)
{
  EXPECT_DOUBLE_EQ(relativeLuminance(0x000000), 0.0);
}

TEST(ContrastTokenPairsTests, RelativeLuminanceOfWhiteIsOne)
{
  EXPECT_DOUBLE_EQ(relativeLuminance(0xFFFFFF), 1.0);
}

TEST(ContrastTokenPairsTests, TitleBarBlackOnCadworkGreyMeetsTextContrast)
{
  EXPECT_GE(contrastRatio(titleBarTextForeground, titleBarBackground), textContrastMinimum);
}

TEST(ContrastTokenPairsTests, TextOnPanelMeetsTextContrast)
{
  EXPECT_GE(contrastRatio(textOnPanelForeground, textOnPanelBackground), textContrastMinimum);
}

TEST(ContrastTokenPairsTests, ComponentChromeMeetsUiContrast)
{
  EXPECT_GE(contrastRatio(componentChromeForeground, componentChromeBackground), uiContrastMinimum);
}

TEST(ContrastTokenPairsTests, ShipsPanelTitleBarAndChromePairs)
{
  const auto pairs = contrastTokenPairs();
  const bool hasTextOnPanel = std::ranges::any_of(pairs, [](const ContrastPair& pair) {
    return pair.name == "textOnPanel";
  });
  const bool hasTitleBar = std::ranges::any_of(pairs, [](const ContrastPair& pair) {
    return pair.name == "titleBarText" && pair.background == titleBarBackground;
  });
  const bool hasChrome = std::ranges::any_of(pairs, [](const ContrastPair& pair) {
    return pair.name == "componentChrome";
  });

  EXPECT_TRUE(hasTextOnPanel && hasTitleBar && hasChrome);
}
