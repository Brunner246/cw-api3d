#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>
#include <string_view>

namespace cw_api3d::tests::fixtures
{

  inline constexpr std::uint32_t textOnPanelForeground = 0x000000;
  inline constexpr std::uint32_t textOnPanelBackground = 0xF0F0F0;
  inline constexpr std::uint32_t titleBarTextForeground = 0x000000;
  inline constexpr std::uint32_t titleBarBackground = 0xD4D4D4;
  inline constexpr std::uint32_t componentChromeForeground = 0x808080;
  inline constexpr std::uint32_t componentChromeBackground = 0xF0F0F0;

  inline constexpr double textContrastMinimum = 4.5;
  inline constexpr double uiContrastMinimum = 3.0;

  struct ContrastPair
  {
    std::string_view name;
    std::uint32_t foreground{};
    std::uint32_t background{};
    double minimumRatio{};
  };

  [[nodiscard]] inline double channelToLinear(const std::uint8_t channel)
  {
    const double srgb = static_cast<double>(channel) / 255.0;
    if (srgb <= 0.04045)
    {
      return srgb / 12.92;
    }
    return std::pow((srgb + 0.055) / 1.055, 2.4);
  }

  [[nodiscard]] inline double relativeLuminance(const std::uint32_t rgb)
  {
    const auto red = static_cast<std::uint8_t>((rgb >> 16) & 0xFFu);
    const auto green = static_cast<std::uint8_t>((rgb >> 8) & 0xFFu);
    const auto blue = static_cast<std::uint8_t>(rgb & 0xFFu);
    return 0.2126 * channelToLinear(red) + 0.7152 * channelToLinear(green)
           + 0.0722 * channelToLinear(blue);
  }

  [[nodiscard]] inline double contrastRatio(const std::uint32_t first, const std::uint32_t second)
  {
    const double firstLuminance = relativeLuminance(first);
    const double secondLuminance = relativeLuminance(second);
    const double lighter = std::max(firstLuminance, secondLuminance);
    const double darker = std::min(firstLuminance, secondLuminance);
    return (lighter + 0.05) / (darker + 0.05);
  }

  [[nodiscard]] inline std::span<const ContrastPair> contrastTokenPairs()
  {
    static constexpr ContrastPair pairs[] = {
      {"textOnPanel", textOnPanelForeground, textOnPanelBackground, textContrastMinimum},
      {"titleBarText", titleBarTextForeground, titleBarBackground, textContrastMinimum},
      {"componentChrome", componentChromeForeground, componentChromeBackground, uiContrastMinimum},
    };
    return pairs;
  }

}
