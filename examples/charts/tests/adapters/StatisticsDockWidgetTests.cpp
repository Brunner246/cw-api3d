#include <gtest/gtest.h>

#include "src/adapters/driving/statistics/HostMainWindowLocator.h"
#include "src/adapters/driving/statistics/StatisticsDockWidget.h"
#include "src/adapters/driving/statistics/StatisticsViewModel.h"
#include "src/application/ActivateElementsUseCase.h"
#include "src/application/FetchElementSnapshotUseCase.h"
#include "tests/doubles/FakeElementActivation.h"
#include "tests/doubles/FakeElementCatalog.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/fixtures/ContrastTokenPairs.h"
#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QDockWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QLibraryInfo>
#include <QMainWindow>
#include <QPointer>
#include <QQuickWidget>
#include <QString>
#include <QVersionNumber>

#include <memory>

using namespace cw_api3d::adapters::driving;
using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;
using namespace cw_api3d::tests::fixtures;

namespace
{

  struct DockHarness
  {
    FakeElementCatalog catalog;
    FakeElementActivation activation;
    FakeLogger logger{LogLevel::Trace};
    FetchElementSnapshotUseCase fetchUseCase;
    ActivateElementsUseCase activateUseCase;
    StatisticsViewModel viewModel;
    QMainWindow host;
    std::unique_ptr<StatisticsDockWidget> dock;

    DockHarness()
      : fetchUseCase(catalog, logger)
      , activateUseCase(activation, logger)
      , viewModel(fetchUseCase, activateUseCase, []() { return false; })
    {
      catalog.setActive(activeSnapshot());
      catalog.setAll(allSnapshot());
      host.resize(800, 600);
      host.show();
      dock = std::make_unique<StatisticsDockWidget>(viewModel, host, &logger);
    }
  };

} // namespace

TEST(StatisticsDockWidgetTests, NullCoreApplicationSkipsUi)
{
  EXPECT_TRUE(shouldSkipStatisticsUi(nullptr));
  EXPECT_FALSE(shouldSkipStatisticsUi(QCoreApplication::instance()));
}

TEST(StatisticsDockWidgetTests, LinksCadlibQt68WithCustomPrefix)
{
  const QString version = QString::fromLatin1(qVersion());
  EXPECT_TRUE(version.startsWith(QStringLiteral("6.8")));

  const QString prefix = QDir::fromNativeSeparators(QLibraryInfo::path(QLibraryInfo::PrefixPath));
  const QString expected = QDir::fromNativeSeparators(QStringLiteral(CW_CUSTOM_QT_PATH));
  EXPECT_TRUE(prefix.startsWith(expected, Qt::CaseInsensitive));
}

TEST(StatisticsDockWidgetTests, ObjectNameAndLeftDockArea)
{
  DockHarness harness;

  auto* found = harness.host.findChild<QDockWidget*>(QStringLiteral("cw_api3d.statisticsDock"));
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(harness.host.dockWidgetArea(found), Qt::LeftDockWidgetArea);
}

TEST(StatisticsDockWidgetTests, FeaturesAllowFloatMoveClose)
{
  DockHarness harness;
  const auto features = harness.dock->features();
  EXPECT_TRUE(features.testFlag(QDockWidget::DockWidgetFloatable));
  EXPECT_TRUE(features.testFlag(QDockWidget::DockWidgetMovable));
  EXPECT_TRUE(features.testFlag(QDockWidget::DockWidgetClosable));
}

TEST(StatisticsDockWidgetTests, FloatAndRedock)
{
  DockHarness harness;
  harness.dock->setFloating(true);
  EXPECT_TRUE(harness.dock->isFloating());
  harness.dock->setFloating(false);
  EXPECT_FALSE(harness.dock->isFloating());
}

TEST(StatisticsDockWidgetTests, MoveToRightDockArea)
{
  DockHarness harness;
  harness.host.addDockWidget(Qt::RightDockWidgetArea, harness.dock.get());
  EXPECT_EQ(harness.host.dockWidgetArea(harness.dock.get()), Qt::RightDockWidgetArea);
}

TEST(StatisticsDockWidgetTests, CloseWithDeleteOnCloseNullsPointer)
{
  DockHarness harness;
  QPointer<StatisticsDockWidget> pointer = harness.dock.get();
  harness.dock.release();
  pointer->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  EXPECT_TRUE(pointer.isNull());
}

TEST(StatisticsDockWidgetTests, TitleAndCadworkGreyStylesheet)
{
  DockHarness harness;
  EXPECT_EQ(harness.dock->windowTitle(), QObject::tr("Model statistics"));
  EXPECT_TRUE(harness.dock->styleSheet().contains(QStringLiteral("#d4d4d4")));
}

TEST(StatisticsDockWidgetTests, ContrastTokensMeetWcag)
{
  for (const auto& pair : contrastTokenPairs())
  {
    EXPECT_GE(contrastRatio(pair.foreground, pair.background), pair.minimumRatio) << pair.name;
  }
}

TEST(StatisticsDockWidgetTests, NonModalAndNotADialog)
{
  DockHarness harness;
  EXPECT_EQ(harness.dock->windowModality(), Qt::NonModal);
  EXPECT_EQ(dynamic_cast<QDialog*>(harness.dock.get()), nullptr);
}

TEST(StatisticsDockWidgetTests, ParentIsHostMainWindowViaLocator)
{
  DockHarness harness;
  EXPECT_EQ(findMainWindow(harness.dock.get()), &harness.host);
  EXPECT_EQ(harness.dock->parentWidget(), &harness.host);
}

TEST(StatisticsDockWidgetTests, ExistingApplicationInstanceUnchanged)
{
  auto* before = QCoreApplication::instance();
  ASSERT_NE(before, nullptr);
  DockHarness harness;
  EXPECT_EQ(QCoreApplication::instance(), before);
}

TEST(StatisticsDockWidgetTests, ViewModelSurvivesDockClose)
{
  DockHarness harness;
  harness.viewModel.setAxis(StatisticsViewModel::Axis::Material);
  harness.viewModel.setChartKind(StatisticsViewModel::ChartKind::Pie);
  harness.viewModel.setUniverse(StatisticsViewModel::Universe::All);

  QPointer<StatisticsDockWidget> pointer = harness.dock.get();
  harness.dock.release();
  pointer->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  ASSERT_TRUE(pointer.isNull());

  auto replacement = std::make_unique<StatisticsDockWidget>(harness.viewModel, harness.host, &harness.logger);
  EXPECT_EQ(replacement->viewModel().universe(), StatisticsViewModel::Universe::All);
  EXPECT_EQ(replacement->viewModel().axis(), StatisticsViewModel::Axis::Material);
  EXPECT_EQ(replacement->viewModel().chartKind(), StatisticsViewModel::ChartKind::Pie);
  EXPECT_NE(static_cast<QObject*>(&replacement->viewModel())->parent(), replacement.get());
}

TEST(StatisticsDockWidgetTests, EscapeDoesNotAcceptDock)
{
  DockHarness harness;
  harness.dock->show();
  QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
  QCoreApplication::sendEvent(harness.dock.get(), &escape);
  EXPECT_TRUE(harness.dock->isVisible());
}

TEST(StatisticsDockWidgetTests, AboutToQuitLeavesNoDanglingDockPointerAfterClose)
{
  DockHarness harness;
  QPointer<StatisticsDockWidget> pointer = harness.dock.get();
  harness.dock->show();
  harness.dock.release();
  pointer->close();
  QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
  EXPECT_TRUE(pointer.isNull());
}

int main(int argc, char** argv)
{
  QApplication application(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
