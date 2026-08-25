#include <gtest/gtest.h>

#include "src/application/ElementStatisticsAggregator.h"
#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <ranges>
#include <set>
#include <string>
#include <vector>

using namespace cw_api3d::application;
using namespace cw_api3d::tests::fixtures;

namespace
{

  [[nodiscard]] const StatisticBucket* bucketByLabel(const StatisticSeries& series, const std::string& label)
  {
    const auto found = std::ranges::find_if(series.buckets, [&](const StatisticBucket& bucket) {
      return bucket.label == label;
    });
    if (found == series.buckets.end())
    {
      return nullptr;
    }
    return &*found;
  }

  [[nodiscard]] std::set<ElementId> idsOf(const StatisticBucket& bucket)
  {
    return {bucket.memberIds.begin(), bucket.memberIds.end()};
  }

  [[nodiscard]] std::map<ElementKind, std::set<ElementId>> expectedTypeIds(const ElementSnapshot& snapshot)
  {
    std::map<ElementKind, std::set<ElementId>> byKind;
    for (const auto& record : snapshot.records)
    {
      byKind[record.kind].insert(record.id);
    }
    return byKind;
  }

}

TEST(ElementStatisticsAggregatorTests, AggregatesTypeAxisOneBucketPerPresentKind)
{
  const auto snapshot = allSnapshot();
  const auto expected = expectedTypeIds(snapshot);
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Type);

  ASSERT_EQ(series.buckets.size(), expected.size());
  for (const auto& bucket : series.buckets)
  {
    const auto matching = std::ranges::find_if(snapshot.records, [&](const ElementRecord& record) {
      return record.kindLabel == bucket.label;
    });
    ASSERT_NE(matching, snapshot.records.end());
    EXPECT_EQ(idsOf(bucket), expected.at(matching->kind));
    EXPECT_EQ(bucket.count, bucket.memberIds.size());
  }
}

TEST(ElementStatisticsAggregatorTests, MaterialAxisKeepsEmptyBucketAndC24Ids)
{
  const auto snapshot = allSnapshot();
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Material);

  const auto* c24 = bucketByLabel(series, "C24");
  ASSERT_NE(c24, nullptr);
  EXPECT_EQ(c24->count, c24MemberCount);
  EXPECT_EQ(c24->memberIds.front(), 1u);
  EXPECT_EQ(c24->memberIds.back(), c24MemberCount);

  const auto* empty = bucketByLabel(series, "");
  ASSERT_NE(empty, nullptr);
  ASSERT_EQ(empty->memberIds.size(), 1u);
  EXPECT_EQ(empty->memberIds.front(), 39u);
}

TEST(ElementStatisticsAggregatorTests, NameAxisCardinalityAtMostTenHasNoOther)
{
  ElementSnapshot snapshot;
  for (ElementId id = 1; id <= 10; ++id)
  {
    snapshot.records.push_back(recordForId(id));
  }
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Name);

  EXPECT_EQ(series.buckets.size(), 10u);
  EXPECT_EQ(bucketByLabel(series, "other"), nullptr);
}

TEST(ElementStatisticsAggregatorTests, NameAxisFifteenUniqueNamesCollapsesToTopTenPlusOther)
{
  const auto snapshot = allSnapshot();
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Name, AggregationPolicy{.topN = 10});

  const auto* empty = bucketByLabel(series, "");
  ASSERT_NE(empty, nullptr);
  ASSERT_EQ(empty->memberIds.size(), 1u);
  EXPECT_EQ(empty->memberIds.front(), 38u);

  const auto* other = bucketByLabel(series, "other");
  ASSERT_NE(other, nullptr);

  std::size_t namedTop = 0;
  for (const auto& bucket : series.buckets)
  {
    if (!bucket.label.empty() && bucket.label != "other")
    {
      ++namedTop;
    }
  }
  EXPECT_EQ(namedTop, 10u);
  EXPECT_EQ(series.buckets.size(), 12u);

  std::set<std::string> otherNames;
  for (const auto id : other->memberIds)
  {
    otherNames.insert(nameForId(id));
  }
  EXPECT_EQ(otherNames.size(), 5u);
}

TEST(ElementStatisticsAggregatorTests, TypeAxisDoesNotCollapseWhenNameWould)
{
  const auto snapshot = allSnapshot();
  const ElementStatisticsAggregator aggregator;
  const auto expected = expectedTypeIds(snapshot);

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Type, AggregationPolicy{.topN = 10});

  EXPECT_EQ(series.buckets.size(), expected.size());
  EXPECT_EQ(bucketByLabel(series, "other"), nullptr);
}

TEST(ElementStatisticsAggregatorTests, LengthAxisHasEightToTwelveBinsLastInclusive)
{
  const auto snapshot = allSnapshot();
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Length);

  std::vector<const StatisticBucket*> numeric;
  for (const auto& bucket : series.buckets)
  {
    if (bucket.lowerEdge.has_value())
    {
      numeric.push_back(&bucket);
    }
  }
  ASSERT_GE(numeric.size(), 8u);
  ASSERT_LE(numeric.size(), 12u);

  double minFinite = 0.0;
  double maxFinite = 0.0;
  bool seenFinite = false;
  std::size_t uniqueFinite = 0;
  std::set<double> values;
  for (const auto& record : snapshot.records)
  {
    if (!record.length.has_value() || !std::isfinite(*record.length))
    {
      continue;
    }
    values.insert(*record.length);
    if (!seenFinite)
    {
      minFinite = *record.length;
      maxFinite = *record.length;
      seenFinite = true;
      continue;
    }
    minFinite = std::min(minFinite, *record.length);
    maxFinite = std::max(maxFinite, *record.length);
  }
  uniqueFinite = values.size();

  EXPECT_LT(numeric.size(), uniqueFinite);
  EXPECT_EQ(*numeric.front()->lowerEdge, minFinite);
  EXPECT_EQ(*numeric.back()->upperEdge, maxFinite);

  bool maxInLast = false;
  for (const auto id : numeric.back()->memberIds)
  {
    const auto length = lengthForId(id);
    if (length.has_value() && *length == maxFinite)
    {
      maxInLast = true;
    }
  }
  EXPECT_TRUE(maxInLast);
}

TEST(ElementStatisticsAggregatorTests, NonFiniteAndMissingLengthGoOnlyToEmptyBucket)
{
  const auto snapshot = allSnapshot();
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Length);

  const auto empty = std::ranges::find_if(series.buckets, [](const StatisticBucket& bucket) {
    return !bucket.lowerEdge.has_value();
  });
  ASSERT_NE(empty, series.buckets.end());
  EXPECT_EQ(idsOf(*empty), (std::set<ElementId>{40, 41}));

  for (const auto& bucket : series.buckets)
  {
    if (!bucket.lowerEdge.has_value())
    {
      continue;
    }
    EXPECT_TRUE(std::ranges::none_of(bucket.memberIds, [](const ElementId id) {
      return id == 40 || id == 41;
    }));
  }
}

TEST(ElementStatisticsAggregatorTests, MinEqualsMaxYieldsSingleDimensionBucket)
{
  ElementSnapshot snapshot;
  snapshot.records.push_back(recordForId(1));
  snapshot.records.push_back(recordForId(1));
  snapshot.records[1].id = 99;
  snapshot.records[1].length = snapshot.records[0].length;
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(snapshot, StatisticAxis::Length);

  ASSERT_EQ(series.buckets.size(), 1u);
  EXPECT_EQ(series.buckets.front().count, 2u);
  EXPECT_EQ(series.buckets.front().lowerEdge, series.buckets.front().upperEdge);
}

TEST(ElementStatisticsAggregatorTests, EmptySnapshotYieldsEmptySeries)
{
  const ElementStatisticsAggregator aggregator;

  const auto series = aggregator.aggregate(ElementSnapshot{}, StatisticAxis::Type);

  EXPECT_TRUE(series.buckets.empty());
}
