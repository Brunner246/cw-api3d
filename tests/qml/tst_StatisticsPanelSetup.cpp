#include "src/adapters/driving/statistics/StatisticsViewModel.h"
#include "src/application/ActivateElementsUseCase.h"
#include "src/application/FetchElementSnapshotUseCase.h"
#include "tests/doubles/FakeElementActivation.h"
#include "tests/doubles/FakeElementCatalog.h"
#include "tests/doubles/FakeLogger.h"
#include "tests/fixtures/ElementSnapshotFixtures.h"

#include <QtQuickTest>
#include <QCoreApplication>
#include <QQmlContext>
#include <QQmlEngine>

using namespace cw_api3d::adapters::driving;
using namespace cw_api3d::application;
using namespace cw_api3d::ports;
using namespace cw_api3d::tests::doubles;
using namespace cw_api3d::tests::fixtures;

class StatisticsPanelTestSetup : public QObject
{
  Q_OBJECT

public:
  StatisticsPanelTestSetup()
    : fetchUseCase(catalog, logger)
    , activateUseCase(activation, logger)
    , viewModel(fetchUseCase, activateUseCase, []() { return false; })
  {
    restoreDefaultFixture();
  }

public slots:
  void qmlEngineAvailable(QQmlEngine* engine)
  {
    engine->addImportPath(QStringLiteral("qrc:/qt/qml"));
    engine->rootContext()->setContextProperty(QStringLiteral("viewModel"), &viewModel);
    engine->rootContext()->setContextProperty(QStringLiteral("testSetup"), this);
  }

  void loadUmlautFixture()
  {
    ElementSnapshot snapshot;
    snapshot.records.push_back(ElementRecord{
      .id = 1,
      .kind = ElementKind::Opening,
      .kindLabel = "Öffnung",
      .name = "Überzug",
      .material = "Grün"});
    catalog.setActive(snapshot);
    viewModel.setUniverse(StatisticsViewModel::Universe::Active);
    viewModel.setAxis(StatisticsViewModel::Axis::Type);
    viewModel.setChartKind(StatisticsViewModel::ChartKind::Bar);
    viewModel.fetch();
    QCoreApplication::processEvents();
  }

  void restoreDefaultFixture()
  {
    catalog.setActive(activeSnapshot());
    catalog.setAll(allSnapshot());
    viewModel.setUniverse(StatisticsViewModel::Universe::Active);
    viewModel.setAxis(StatisticsViewModel::Axis::Type);
    viewModel.setChartKind(StatisticsViewModel::ChartKind::Bar);
    viewModel.fetch();
    QCoreApplication::processEvents();
  }

private:
  FakeElementCatalog catalog;
  FakeElementActivation activation;
  FakeLogger logger{LogLevel::Trace};
  FetchElementSnapshotUseCase fetchUseCase;
  ActivateElementsUseCase activateUseCase;
  StatisticsViewModel viewModel;
};

QUICK_TEST_MAIN_WITH_SETUP(tst_StatisticsPanel, StatisticsPanelTestSetup)

#include "tst_StatisticsPanelSetup.moc"
