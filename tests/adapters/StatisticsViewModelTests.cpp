#include <gtest/gtest.h>

#include "src/adapters/driving/statistics/StatisticsViewModel.h"
#include "src/application/ActivateElementsUseCase.h"
#include "src/application/ElementStatisticsAggregator.h"
#include "src/application/FetchElementSnapshotUseCase.h"
#include "tests/doubles/FakeElementActivation.h"
#include "tests/doubles/FakeElementCatalog.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <QCoreApplication>
#include <QLocale>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cmath>
#include <expected>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace cw_api3d::adapters::driving;
using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;
using namespace cw_api3d::tests::fixtures;

namespace
{

  [[nodiscard]] std::set<ElementId> idsOf(const ElementSnapshot& snapshot)
  {
    std::set<ElementId> ids;
    for (const auto& record : snapshot.records)
    {
      ids.insert(record.id);
    }
    return ids;
  }

  [[nodiscard]] std::set<ElementId> idsInModel(const BucketListModel& model)
  {
    std::set<ElementId> ids;
    for (int row = 0; row < model.rowCount(); ++row)
    {
      for (const auto id : model.memberIdsAt(row))
      {
        ids.insert(id);
      }
    }
    return ids;
  }

  [[nodiscard]] int rowWithLabel(const BucketListModel& model, const QString& label)
  {
    for (int row = 0; row < model.rowCount(); ++row)
    {
      if (model.data(model.index(row), BucketListModel::LabelRole).toString() == label)
      {
        return row;
      }
    }
    return -1;
  }

  [[nodiscard]] QString labelAt(const BucketListModel& model, const int row)
  {
    return model.data(model.index(row), BucketListModel::LabelRole).toString();
  }

  [[nodiscard]] QStringList allLabels(const BucketListModel& model)
  {
    QStringList labels;
    labels.reserve(model.rowCount());
    for (int row = 0; row < model.rowCount(); ++row)
    {
      labels.push_back(labelAt(model, row));
    }
    return labels;
  }

  struct ViewModelHarness
  {
    FakeElementCatalog catalog;
    FakeElementActivation activation;
    FakeLogger logger{LogLevel::Trace};
    FetchElementSnapshotUseCase fetchUseCase;
    ActivateElementsUseCase activateUseCase;
    StatisticsViewModel viewModel;

    explicit ViewModelHarness(std::function<bool()> clientAreaAnimationEnabled = {})
      : fetchUseCase(catalog, logger)
      , activateUseCase(activation, logger)
      , viewModel(fetchUseCase, activateUseCase, std::move(clientAreaAnimationEnabled))
    {
      catalog.setActive(activeSnapshot());
      catalog.setAll(allSnapshot());
    }

    void fetchAndWait()
    {
      viewModel.fetch();
      QCoreApplication::processEvents();
    }
  };

  class BusyRecordingCatalog : public FakeElementCatalog
  {
  public:
    StatisticsViewModel* viewModel{nullptr};
    mutable bool busyAtEntry{false};

    [[nodiscard]] std::expected<ElementSnapshot, std::string> fetch(const ElementUniverse universe) const override
    {
      if (viewModel != nullptr)
      {
        busyAtEntry = viewModel->busy();
      }
      return FakeElementCatalog::fetch(universe);
    }
  };

}

TEST(StatisticsViewModelTests, FetchDefaultUniverseMatchesActiveFixture)
{
  ViewModelHarness harness;

  harness.fetchAndWait();

  EXPECT_EQ(idsInModel(*harness.viewModel.buckets()), idsOf(activeSnapshot()));
  EXPECT_EQ(harness.viewModel.totalCount(), static_cast<int>(activeSnapshot().records.size()));
  EXPECT_EQ(harness.catalog.callCount(), 1);
}

TEST(StatisticsViewModelTests, SetUniverseActiveThenFetchMatchesActiveIds)
{
  ViewModelHarness harness;
  harness.viewModel.setUniverse(StatisticsViewModel::Universe::Active);

  harness.fetchAndWait();

  EXPECT_EQ(idsInModel(*harness.viewModel.buckets()), idsOf(activeSnapshot()));
}

TEST(StatisticsViewModelTests, SetUniverseAllThenFetchMatchesAllIds)
{
  ViewModelHarness harness;
  harness.viewModel.setUniverse(StatisticsViewModel::Universe::All);

  harness.fetchAndWait();

  EXPECT_EQ(idsInModel(*harness.viewModel.buckets()), idsOf(allSnapshot()));
  EXPECT_EQ(harness.viewModel.totalCount(), static_cast<int>(allSnapshot().records.size()));
}

TEST(StatisticsViewModelTests, EmptyActiveSnapshotSetsEmptyNotError)
{
  ViewModelHarness harness;
  harness.catalog.setActive({});

  harness.fetchAndWait();

  EXPECT_TRUE(harness.viewModel.empty());
  EXPECT_EQ(harness.viewModel.buckets()->rowCount(), 0);
  EXPECT_TRUE(harness.viewModel.errorMessage().isEmpty());
}

TEST(StatisticsViewModelTests, EmptyAllSnapshotSetsEmptyNotError)
{
  ViewModelHarness harness;
  harness.catalog.setAll({});
  harness.viewModel.setUniverse(StatisticsViewModel::Universe::All);

  harness.fetchAndWait();

  EXPECT_TRUE(harness.viewModel.empty());
  EXPECT_EQ(harness.viewModel.buckets()->rowCount(), 0);
  EXPECT_TRUE(harness.viewModel.errorMessage().isEmpty());
}

TEST(StatisticsViewModelTests, BusyTrueAtCatalogEntryFalseAfterSingleShot)
{
  BusyRecordingCatalog catalog;
  catalog.setActive(activeSnapshot());
  FakeElementActivation activation;
  FakeLogger logger{LogLevel::Trace};
  FetchElementSnapshotUseCase fetchUseCase(catalog, logger);
  ActivateElementsUseCase activateUseCase(activation, logger);
  StatisticsViewModel viewModel(fetchUseCase, activateUseCase);
  catalog.viewModel = &viewModel;

  viewModel.fetch();
  EXPECT_TRUE(viewModel.busy());
  EXPECT_EQ(catalog.callCount(), 0);

  QCoreApplication::processEvents();

  EXPECT_TRUE(catalog.busyAtEntry);
  EXPECT_FALSE(viewModel.busy());
}

TEST(StatisticsViewModelTests, HostFailureSetsErrorMessageWithoutThrowing)
{
  ViewModelHarness harness;
  harness.catalog.setFailure("host failed");

  harness.fetchAndWait();

  EXPECT_EQ(harness.viewModel.errorMessage().toStdString(), "host failed");
  EXPECT_FALSE(harness.viewModel.empty());
}

TEST(StatisticsViewModelTests, SetAxisReaggregatesWithoutSecondCatalogFetch)
{
  ViewModelHarness harness;
  harness.fetchAndWait();
  const auto typeLabels = allLabels(*harness.viewModel.buckets());

  harness.viewModel.setAxis(StatisticsViewModel::Axis::Material);
  const auto materialLabels = allLabels(*harness.viewModel.buckets());
  harness.viewModel.setAxis(StatisticsViewModel::Axis::Name);
  const auto nameLabels = allLabels(*harness.viewModel.buckets());

  EXPECT_EQ(harness.catalog.callCount(), 1);
  EXPECT_NE(materialLabels, typeLabels);
  EXPECT_NE(nameLabels, materialLabels);
}

TEST(StatisticsViewModelTests, EmptyAttributeDisplayRoleIsNonEmptyTrLabel)
{
  ViewModelHarness harness;
  harness.fetchAndWait();
  harness.viewModel.setAxis(StatisticsViewModel::Axis::Name);

  const auto emptyRow = rowWithLabel(*harness.viewModel.buckets(), QStringLiteral("Empty"));
  ASSERT_GE(emptyRow, 0);
  EXPECT_FALSE(labelAt(*harness.viewModel.buckets(), emptyRow).isEmpty());
  EXPECT_EQ(harness.viewModel.buckets()->memberIdsAt(emptyRow), std::vector<ElementId>{38});
}

TEST(StatisticsViewModelTests, DimensionLabelsUsePinnedGermanLocaleAndUnitSuffix)
{
  ViewModelHarness harness;
  harness.viewModel.setLocale(QLocale(QLocale::German));
  harness.fetchAndWait();
  harness.viewModel.setAxis(StatisticsViewModel::Axis::Length);

  const auto series = ElementStatisticsAggregator{}.aggregate(activeSnapshot(), StatisticAxis::Length);
  const QLocale german{QLocale::German};
  const QLocale english{QLocale::English};
  ASSERT_EQ(harness.viewModel.buckets()->rowCount(), static_cast<int>(series.buckets.size()));

  bool sawFormattedEdge = false;
  for (int row = 0; row < harness.viewModel.buckets()->rowCount(); ++row)
  {
    const auto& bucket = series.buckets[static_cast<std::size_t>(row)];
    if (!bucket.lowerEdge.has_value() || !bucket.upperEdge.has_value())
    {
      continue;
    }
    const auto label = labelAt(*harness.viewModel.buckets(), row);
    const auto germanLower = german.toString(static_cast<qlonglong>(*bucket.lowerEdge));
    const auto englishLower = english.toString(static_cast<qlonglong>(*bucket.lowerEdge));
    EXPECT_TRUE(label.contains(germanLower));
    EXPECT_TRUE(label.contains(QStringLiteral("mm")));
    if (germanLower != englishLower)
    {
      EXPECT_FALSE(label.contains(englishLower));
    }
    sawFormattedEdge = true;
  }
  EXPECT_TRUE(sawFormattedEdge);
}

TEST(StatisticsViewModelTests, BucketCountEqualsMemberSizeAndShareIsCountOverTotal)
{
  ViewModelHarness harness;
  harness.fetchAndWait();
  const auto* model = harness.viewModel.buckets();
  const auto total = harness.viewModel.totalCount();
  ASSERT_GT(total, 0);
  ASSERT_GT(model->rowCount(), 0);

  const auto row = 0;
  const auto count = model->data(model->index(row), BucketListModel::CountRole).toInt();
  const auto share = model->data(model->index(row), BucketListModel::ShareRole).toDouble();
  EXPECT_EQ(count, static_cast<int>(model->memberIdsAt(row).size()));
  EXPECT_DOUBLE_EQ(share, static_cast<double>(count) / static_cast<double>(total));
}

TEST(StatisticsViewModelTests, SelectBucketActivatesThatBucketsMemberIds)
{
  ViewModelHarness harness;
  harness.fetchAndWait();

  harness.viewModel.selectBucket(0);

  EXPECT_EQ(harness.activation.lastIds(), harness.viewModel.buckets()->memberIdsAt(0));
}

TEST(StatisticsViewModelTests, SelectBucketOnC24ActivatesFullMemberSet)
{
  ViewModelHarness harness;
  harness.viewModel.setUniverse(StatisticsViewModel::Universe::All);
  harness.fetchAndWait();
  harness.viewModel.setAxis(StatisticsViewModel::Axis::Material);
  const auto c24Row = rowWithLabel(*harness.viewModel.buckets(), QStringLiteral("C24"));
  ASSERT_GE(c24Row, 0);

  harness.viewModel.selectBucket(c24Row);

  EXPECT_EQ(harness.activation.lastIds().size(), c24MemberCount);
  EXPECT_EQ(harness.activation.lastIds(), harness.viewModel.buckets()->memberIdsAt(c24Row));
}

TEST(StatisticsViewModelTests, SelectBucketOnEmptyAttributeActivatesThoseIds)
{
  ViewModelHarness harness;
  harness.fetchAndWait();
  harness.viewModel.setAxis(StatisticsViewModel::Axis::Material);
  const auto emptyRow = rowWithLabel(*harness.viewModel.buckets(), QStringLiteral("Empty"));
  ASSERT_GE(emptyRow, 0);

  harness.viewModel.selectBucket(emptyRow);

  EXPECT_EQ(harness.activation.lastIds(), std::vector<ElementId>{39});
}

TEST(StatisticsViewModelTests, SecondFetchSeesMutatedCatalogSnapshot)
{
  ViewModelHarness harness;
  harness.fetchAndWait();
  auto mutated = activeSnapshot();
  mutated.records = {recordForId(1)};
  harness.catalog.setActive(std::move(mutated));

  harness.fetchAndWait();

  EXPECT_EQ(harness.viewModel.totalCount(), 1);
  EXPECT_EQ(idsInModel(*harness.viewModel.buckets()), std::set<ElementId>{1});
}

TEST(StatisticsViewModelTests, AnimationsDisabledWhenClientAreaAnimationOff)
{
  ViewModelHarness harness([] { return false; });

  EXPECT_FALSE(harness.viewModel.animationsEnabled());
  EXPECT_EQ(harness.viewModel.chartTransitionDuration(), 0);
}

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
