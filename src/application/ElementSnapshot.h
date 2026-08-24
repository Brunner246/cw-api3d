#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cw_api3d::application
{

  using ElementId = std::uint64_t;

  enum class ElementUniverse
  {
    Active,
    All
  };

  enum class ElementKind
  {
    FramedWall,
    SolidWoodWall,
    LogWall,
    Wall,
    RectangularBeam,
    CircularBeam,
    Beam,
    Panel,
    Opening,
    Other
  };

  struct ElementRecord
  {
    ElementId id{};
    ElementKind kind{ElementKind::Other};
    std::string kindLabel;
    std::string name;
    std::string material;
    std::optional<double> length;
    std::optional<double> width;
    std::optional<double> height;
    std::optional<double> volume;
  };

  struct ElementSnapshot
  {
    std::vector<ElementRecord> records;
  };

  [[nodiscard]] constexpr std::string_view toString(const ElementUniverse universe) noexcept
  {
    switch (universe)
    {
      case ElementUniverse::Active:
        return "Active";
      case ElementUniverse::All:
        return "All";
    }
    return "Unknown";
  }

} // namespace cw_api3d::application
