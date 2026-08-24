#pragma once

#include "src/application/ElementSnapshot.h"
#include "src/application/StatisticSeries.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace cw_api3d::application
{

  class ElementStatisticsAggregator
  {
  public:
    [[nodiscard]] StatisticSeries aggregate(
      const ElementSnapshot& snapshot,
      const StatisticAxis axis,
      const AggregationPolicy& policy = {}) const
    {
      switch (axis)
      {
        case StatisticAxis::Type:
          return aggregateByType(snapshot);
        case StatisticAxis::Material:
          return collapseStringAxis(snapshot, axis, policy, [](const ElementRecord& record) {
            return record.material;
          });
        case StatisticAxis::Name:
          return collapseStringAxis(snapshot, axis, policy, [](const ElementRecord& record) {
            return record.name;
          });
        case StatisticAxis::Length:
          return aggregateDimension(snapshot, axis, [](const ElementRecord& record) {
            return record.length;
          });
        case StatisticAxis::Width:
          return aggregateDimension(snapshot, axis, [](const ElementRecord& record) {
            return record.width;
          });
        case StatisticAxis::Height:
          return aggregateDimension(snapshot, axis, [](const ElementRecord& record) {
            return record.height;
          });
        case StatisticAxis::Volume:
          return aggregateDimension(snapshot, axis, [](const ElementRecord& record) {
            return record.volume;
          });
      }
      return StatisticSeries{.axis = axis};
    }

  private:
    static constexpr std::string_view otherLabel = "other";

    [[nodiscard]] static StatisticSeries aggregateByType(const ElementSnapshot& snapshot)
    {
      StatisticSeries series{.axis = StatisticAxis::Type};
      std::map<ElementKind, std::size_t> indexByKind;
      for (const auto& record : snapshot.records)
      {
        const auto found = indexByKind.find(record.kind);
        if (found == indexByKind.end())
        {
          indexByKind.emplace(record.kind, series.buckets.size());
          series.buckets.push_back(StatisticBucket{
            .label = record.kindLabel,
            .count = 1,
            .memberIds = {record.id}});
          continue;
        }
        auto& bucket = series.buckets[found->second];
        bucket.memberIds.push_back(record.id);
        bucket.count = bucket.memberIds.size();
      }
      return series;
    }

    template<typename KeySelector>
    [[nodiscard]] static StatisticSeries collapseStringAxis(
      const ElementSnapshot& snapshot,
      const StatisticAxis axis,
      const AggregationPolicy& policy,
      KeySelector&& keyOf)
    {
      StatisticSeries series{.axis = axis};
      std::map<std::string, std::size_t> indexByKey;
      for (const auto& record : snapshot.records)
      {
        const std::string key = keyOf(record);
        const auto found = indexByKey.find(key);
        if (found == indexByKey.end())
        {
          indexByKey.emplace(key, series.buckets.size());
          series.buckets.push_back(StatisticBucket{
            .label = key,
            .count = 1,
            .memberIds = {record.id}});
          continue;
        }
        auto& bucket = series.buckets[found->second];
        bucket.memberIds.push_back(record.id);
        bucket.count = bucket.memberIds.size();
      }

      std::optional<StatisticBucket> emptyBucket;
      std::vector<StatisticBucket> ranked;
      ranked.reserve(series.buckets.size());
      for (auto& bucket : series.buckets)
      {
        if (bucket.label.empty())
        {
          emptyBucket = std::move(bucket);
          continue;
        }
        ranked.push_back(std::move(bucket));
      }

      series.buckets.clear();
      if (emptyBucket.has_value())
      {
        series.buckets.push_back(std::move(*emptyBucket));
      }

      if (ranked.size() <= policy.topN)
      {
        series.buckets.insert(series.buckets.end(), ranked.begin(), ranked.end());
        return series;
      }

      std::ranges::sort(ranked, [](const StatisticBucket& lhs, const StatisticBucket& rhs) {
        if (lhs.count != rhs.count)
        {
          return lhs.count > rhs.count;
        }
        return lhs.label < rhs.label;
      });

      StatisticBucket other{
        .label = std::string{otherLabel}};
      for (std::size_t i = 0; i < ranked.size(); ++i)
      {
        if (i < policy.topN)
        {
          series.buckets.push_back(std::move(ranked[i]));
          continue;
        }
        other.memberIds.insert(other.memberIds.end(), ranked[i].memberIds.begin(), ranked[i].memberIds.end());
      }
      other.count = other.memberIds.size();
      series.buckets.push_back(std::move(other));
      return series;
    }

    template<typename ValueSelector>
    [[nodiscard]] static StatisticSeries aggregateDimension(
      const ElementSnapshot& snapshot,
      const StatisticAxis axis,
      ValueSelector&& valueOf)
    {
      StatisticSeries series{.axis = axis};
      std::vector<std::pair<double, ElementId>> finite;
      StatisticBucket empty;
      for (const auto& record : snapshot.records)
      {
        const std::optional<double> value = valueOf(record);
        if (!value.has_value() || !std::isfinite(*value))
        {
          empty.memberIds.push_back(record.id);
          continue;
        }
        finite.emplace_back(*value, record.id);
      }
      empty.count = empty.memberIds.size();

      if (finite.empty())
      {
        if (empty.count > 0)
        {
          series.buckets.push_back(std::move(empty));
        }
        return series;
      }

      double min = finite.front().first;
      double max = finite.front().first;
      for (const auto& [value, _] : finite)
      {
        min = std::min(min, value);
        max = std::max(max, value);
      }

      if (min == max)
      {
        StatisticBucket single{
          .count = finite.size(),
          .lowerEdge = min,
          .upperEdge = max};
        single.memberIds.reserve(finite.size());
        for (const auto& [_, id] : finite)
        {
          single.memberIds.push_back(id);
        }
        series.buckets.push_back(std::move(single));
        if (empty.count > 0)
        {
          series.buckets.push_back(std::move(empty));
        }
        return series;
      }

      const auto edges = niceEdges(min, max);
      std::vector<StatisticBucket> bins(edges.size() - 1);
      for (std::size_t i = 0; i < bins.size(); ++i)
      {
        bins[i].lowerEdge = edges[i];
        bins[i].upperEdge = edges[i + 1];
      }

      const auto lastIndex = bins.size() - 1;
      for (const auto& [value, id] : finite)
      {
        std::size_t index = lastIndex;
        for (std::size_t i = 0; i < lastIndex; ++i)
        {
          if (value >= edges[i] && value < edges[i + 1])
          {
            index = i;
            break;
          }
        }
        bins[index].memberIds.push_back(id);
      }

      for (auto& bin : bins)
      {
        bin.count = bin.memberIds.size();
        series.buckets.push_back(std::move(bin));
      }
      if (empty.count > 0)
      {
        series.buckets.push_back(std::move(empty));
      }
      return series;
    }

    [[nodiscard]] static double niceNumber(const double value, const bool round)
    {
      if (!(value > 0.0) || !std::isfinite(value))
      {
        return 1.0;
      }
      const double exponent = std::floor(std::log10(value));
      const double fraction = value / std::pow(10.0, exponent);
      double niceFraction = 10.0;
      if (round)
      {
        if (fraction < 1.5)
        {
          niceFraction = 1.0;
        }
        else if (fraction < 3.0)
        {
          niceFraction = 2.0;
        }
        else if (fraction < 7.0)
        {
          niceFraction = 5.0;
        }
      }
      else if (fraction <= 1.0)
      {
        niceFraction = 1.0;
      }
      else if (fraction <= 2.0)
      {
        niceFraction = 2.0;
      }
      else if (fraction <= 5.0)
      {
        niceFraction = 5.0;
      }
      return niceFraction * std::pow(10.0, exponent);
    }

    [[nodiscard]] static double nextNiceUp(const double width)
    {
      return niceNumber(width * 2.0, true);
    }

    [[nodiscard]] static double nextNiceDown(const double width)
    {
      return niceNumber(width / 2.0, true);
    }

    [[nodiscard]] static std::size_t binCount(const double min, const double max, const double width)
    {
      if (!(width > 0.0))
      {
        return 1;
      }
      const auto count = static_cast<std::size_t>(std::ceil((max - min) / width));
      return std::max<std::size_t>(1, count);
    }

    [[nodiscard]] static std::vector<double> niceEdges(const double min, const double max)
    {
      double width = niceNumber((max - min) / 10.0, true);
      auto count = binCount(min, max, width);
      while (count > 12)
      {
        const double wider = nextNiceUp(width);
        if (wider <= width)
        {
          break;
        }
        width = wider;
        count = binCount(min, max, width);
      }
      while (count < 8)
      {
        const double narrower = nextNiceDown(width);
        if (narrower >= width || narrower <= 0.0)
        {
          break;
        }
        const auto nextCount = binCount(min, max, narrower);
        if (nextCount > 12)
        {
          break;
        }
        width = narrower;
        count = nextCount;
      }

      std::vector<double> edges;
      edges.reserve(count + 1);
      edges.push_back(min);
      for (std::size_t i = 1; i < count; ++i)
      {
        edges.push_back(min + static_cast<double>(i) * width);
      }
      edges.push_back(max);
      return edges;
    }
  };

} // namespace cw_api3d::application
