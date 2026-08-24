#pragma once

#include "src/application/ElementSnapshot.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace cw_api3d::application
{

  enum class StatisticAxis
  {
    Type,
    Material,
    Name,
    Length,
    Width,
    Height,
    Volume
  };

  struct AggregationPolicy
  {
    std::size_t topN{10};
  };

  struct StatisticBucket
  {
    std::string label;
    std::size_t count{};
    std::vector<ElementId> memberIds;
    std::optional<double> lowerEdge;
    std::optional<double> upperEdge;
  };

  struct StatisticSeries
  {
    StatisticAxis axis{};
    std::vector<StatisticBucket> buckets;
  };

} // namespace cw_api3d::application
