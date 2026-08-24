#pragma once

#include "src/application/ElementSnapshot.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace cw_api3d::tests::fixtures
{

  using application::ElementId;
  using application::ElementKind;
  using application::ElementRecord;
  using application::ElementSnapshot;

  inline constexpr std::size_t c24MemberCount = 32;
  inline constexpr std::size_t uniqueNameCount = 15;
  inline constexpr ElementId lastActiveId = 40;
  inline constexpr ElementId lastAllId = 50;

  [[nodiscard]] inline std::string uniqueName(const std::size_t oneBasedIndex)
  {
    return std::format("Name{:02}", oneBasedIndex);
  }

  [[nodiscard]] inline const char* kindLabelOf(const ElementKind kind) noexcept
  {
    switch (kind)
    {
      case ElementKind::RectangularBeam:
        return "Rectangular beam";
      case ElementKind::CircularBeam:
        return "Circular beam";
      case ElementKind::Panel:
        return "Panel";
      case ElementKind::Opening:
        return "Opening";
      case ElementKind::FramedWall:
        return "Framed wall";
      case ElementKind::SolidWoodWall:
        return "Solid wood wall";
      case ElementKind::LogWall:
        return "Log wall";
      case ElementKind::Wall:
        return "Wall";
      case ElementKind::Beam:
        return "Beam";
      case ElementKind::Other:
        return "Other";
    }
    return "Other";
  }

  [[nodiscard]] inline ElementKind kindForId(const ElementId id) noexcept
  {
    if (id <= c24MemberCount)
    {
      return ElementKind::RectangularBeam;
    }

    switch (id)
    {
      case 33:
      case 37:
      case 42:
        return ElementKind::Panel;
      case 34:
      case 43:
        return ElementKind::CircularBeam;
      case 35:
      case 40:
      case 44:
        return ElementKind::Opening;
      case 36:
      case 41:
      case 45:
        return ElementKind::Other;
      default:
        return ElementKind::Panel;
    }
  }

  [[nodiscard]] inline std::string nameForId(const ElementId id)
  {
    if (id == 38)
    {
      return {};
    }
    if (id <= c24MemberCount)
    {
      return uniqueName(((id - 1) % 10) + 1);
    }
    static constexpr std::array<std::size_t, 18> extraNames{
      11,
      12,
      13,
      14,
      15,
      1,
      2,
      3,
      4,
      5,
      6,
      7,
      8,
      9,
      10,
      11,
      12,
      13};
    const auto extraIndex = static_cast<std::size_t>(id - (c24MemberCount + 1));
    return uniqueName(extraNames.at(extraIndex));
  }

  [[nodiscard]] inline std::string materialForId(const ElementId id)
  {
    if (id <= c24MemberCount)
    {
      return "C24";
    }
    if (id == 39)
    {
      return {};
    }
    switch (kindForId(id))
    {
      case ElementKind::Panel:
        return "OSB";
      case ElementKind::CircularBeam:
        return "S355";
      case ElementKind::Opening:
        return "void";
      case ElementKind::Other:
        return "misc";
      case ElementKind::RectangularBeam:
        return "GL24h";
      default:
        return "misc";
    }
  }

  [[nodiscard]] inline std::optional<double> lengthForId(const ElementId id)
  {
    if (id == 40)
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
    if (id == 41)
    {
      return std::nullopt;
    }
    return 1000.0 + static_cast<double>(id - 1) * 180.0;
  }

  [[nodiscard]] inline ElementRecord recordForId(const ElementId id)
  {
    const auto kind = kindForId(id);
    const auto length = lengthForId(id);
    const auto hasFiniteLength = length.has_value() && std::isfinite(*length);
    return ElementRecord{
      .id = id,
      .kind = kind,
      .kindLabel = kindLabelOf(kind),
      .name = nameForId(id),
      .material = materialForId(id),
      .length = length,
      .width = hasFiniteLength ? std::optional<double>{100.0 + static_cast<double>(id)} : length,
      .height = hasFiniteLength ? std::optional<double>{200.0 + static_cast<double>(id)} : length,
      .volume = hasFiniteLength ? std::optional<double>{(*length) * 0.02} : length,
    };
  }

  [[nodiscard]] inline ElementSnapshot allSnapshot()
  {
    ElementSnapshot snapshot;
    snapshot.records.reserve(static_cast<std::size_t>(lastAllId));
    for (ElementId id = 1; id <= lastAllId; ++id)
    {
      snapshot.records.push_back(recordForId(id));
    }
    return snapshot;
  }

  [[nodiscard]] inline ElementSnapshot activeSnapshot()
  {
    ElementSnapshot snapshot;
    snapshot.records.reserve(static_cast<std::size_t>(lastActiveId));
    for (ElementId id = 1; id <= lastActiveId; ++id)
    {
      snapshot.records.push_back(recordForId(id));
    }
    return snapshot;
  }

}
