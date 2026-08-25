#include <gtest/gtest.h>

#include "src/adapters/driving/statistics/BucketListModel.h"
#include "src/adapters/driving/statistics/CategoricalPalette.h"
#include "src/application/StatisticSeries.h"

#include <QColor>
#include <QCoreApplication>
#include <QString>
#include <QVariant>
#include <QVector>

#include <cstddef>
#include <vector>

using cw_api3d::adapters::driving::BucketListModel;
using cw_api3d::adapters::driving::CategoricalPalette;
using cw_api3d::application::StatisticBucket;

TEST(CategoricalPaletteTests, SizeIsTwelve)
{
  EXPECT_EQ(CategoricalPalette::size(), 12);
}

TEST(CategoricalPaletteTests, FirstTwelveEntriesAreUnique)
{
  for (std::size_t i = 0; i < CategoricalPalette::size(); ++i)
  {
    for (std::size_t j = i + 1; j < CategoricalPalette::size(); ++j)
    {
      EXPECT_NE(CategoricalPalette::colorAt(i), CategoricalPalette::colorAt(j)) << "i=" << i << " j=" << j;
    }
  }
}

TEST(CategoricalPaletteTests, ColorAtWrapsAtPaletteLength)
{
  EXPECT_EQ(CategoricalPalette::colorAt(12), CategoricalPalette::colorAt(0));
}

TEST(CategoricalPaletteTests, ColorAtWrapsPastPaletteLength)
{
  EXPECT_EQ(CategoricalPalette::colorAt(13), CategoricalPalette::colorAt(1));
}

TEST(CategoricalPaletteTests, ColorAtMatchesLockedHueTable)
{
  EXPECT_EQ(CategoricalPalette::colorAt(0), QColor{QStringLiteral("#4E79A7")});
  EXPECT_EQ(CategoricalPalette::colorAt(1), QColor{QStringLiteral("#F28E2B")});
  EXPECT_EQ(CategoricalPalette::colorAt(11), QColor{QStringLiteral("#D37295")});
}

namespace
{

  void populate(BucketListModel& model, const int rowCount)
  {
    std::vector<StatisticBucket> buckets(static_cast<std::size_t>(rowCount));
    QVector<QString> labels;
    labels.reserve(rowCount);
    std::size_t total = 0;
    for (int row = 0; row < rowCount; ++row)
    {
      buckets[static_cast<std::size_t>(row)].label = "bucket";
      buckets[static_cast<std::size_t>(row)].count = static_cast<std::size_t>(row + 1);
      total += buckets[static_cast<std::size_t>(row)].count;
      labels.push_back(QStringLiteral("L%1").arg(row));
    }
    model.setBuckets(std::move(buckets), total, std::move(labels));
  }

} // namespace

TEST(CategoricalPaletteTests, ColorAtEqualsPaletteForPopulatedRows)
{
  BucketListModel model;
  populate(model, 3);

  EXPECT_EQ(model.colorAt(0), CategoricalPalette::colorAt(0));
  EXPECT_EQ(model.colorAt(1), CategoricalPalette::colorAt(1));
  EXPECT_EQ(model.colorAt(2), CategoricalPalette::colorAt(2));
}

TEST(CategoricalPaletteTests, ColorRoleEqualsPaletteForPopulatedRows)
{
  BucketListModel model;
  populate(model, 2);
  const auto color = model.data(model.index(1), BucketListModel::ColorRole);

  EXPECT_EQ(color.value<QColor>(), CategoricalPalette::colorAt(1));
}

TEST(CategoricalPaletteTests, ColorRoleNameIsColor)
{
  BucketListModel model;

  EXPECT_EQ(model.roleNames().value(BucketListModel::ColorRole), QByteArrayLiteral("color"));
}

TEST(CategoricalPaletteTests, ColorAtInvalidRowIsInvalid)
{
  BucketListModel model;
  populate(model, 1);

  EXPECT_FALSE(model.colorAt(-1).isValid());
  EXPECT_FALSE(model.colorAt(1).isValid());
}

TEST(CategoricalPaletteTests, ColorRoleInvalidRowIsEmpty)
{
  BucketListModel model;
  populate(model, 1);
  const auto color = model.data(model.index(5), BucketListModel::ColorRole);

  EXPECT_FALSE(color.isValid());
}

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
