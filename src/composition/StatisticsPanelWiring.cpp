#include "src/composition/StatisticsPanelWiring.h"

#include "src/adapters/driven/cadwork/CadworkElementActivationAdapter.h"
#include "src/adapters/driven/cadwork/CadworkElementCatalogAdapter.h"
#include "src/adapters/driving/statistics/HostMainWindowLocator.h"
#include "src/adapters/driving/statistics/StatisticsDockWidget.h"
#include "src/adapters/driving/statistics/StatisticsViewModel.h"
#include "src/application/ActivateElementsUseCase.h"
#include "src/application/FetchElementSnapshotUseCase.h"
#include "src/composition/HostAbsentQtRuntimeVisibility.h"
#include "src/composition/PluginUiSession.h"

#include <cwapi3d/ICwAPI3DControllerFactory.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

#include <QCoreApplication>
#include <QPointer>

#include <memory>

namespace cw_api3d::composition
{
  namespace
  {
    struct ProductionGraph
    {
      ports::LoggerPtr logger;
      adapters::driven::cadwork::CadworkElementCatalogHost catalogHost;
      adapters::driven::cadwork::CadworkElementActivationHost activationHost;
      std::unique_ptr<adapters::driven::cadwork::CadworkElementCatalogAdapter> catalog;
      std::unique_ptr<adapters::driven::cadwork::CadworkElementActivationAdapter> activation;
      std::unique_ptr<application::FetchElementSnapshotUseCase> fetch;
      std::unique_ptr<application::ActivateElementsUseCase> activate;
      std::unique_ptr<adapters::driving::StatisticsViewModel> viewModel;
      QPointer<adapters::driving::StatisticsDockWidget> dock;
    };

    std::unique_ptr<ProductionGraph> gGraph;

    void ensureGraph(CwAPI3D::ControllerFactory* factory, const ports::LoggerPtr& logger)
    {
      if (gGraph)
      {
        if (logger)
        {
          gGraph->logger = logger;
        }
        return;
      }

      gGraph = std::make_unique<ProductionGraph>();
      gGraph->logger = logger;
      if (factory == nullptr)
      {
        return;
      }

      gGraph->catalogHost.elements = factory->getElementController();
      gGraph->catalogHost.attributes = factory->getAttributeController();
      gGraph->catalogHost.geometry = factory->getGeometryController();
      gGraph->activationHost.factory = factory;
      gGraph->activationHost.visualization = factory->getVisualizationController();
      gGraph->catalog = std::make_unique<adapters::driven::cadwork::CadworkElementCatalogAdapter>(
        &gGraph->catalogHost,
        gGraph->logger);
      gGraph->activation = std::make_unique<adapters::driven::cadwork::CadworkElementActivationAdapter>(
        &gGraph->activationHost,
        gGraph->logger);
      gGraph->fetch = std::make_unique<application::FetchElementSnapshotUseCase>(
        *gGraph->catalog,
        *gGraph->logger);
      gGraph->activate = std::make_unique<application::ActivateElementsUseCase>(
        *gGraph->activation,
        *gGraph->logger);
      gGraph->viewModel = std::make_unique<adapters::driving::StatisticsViewModel>(
        *gGraph->fetch,
        *gGraph->activate);
    }

    IStatisticsPanel* createOrFocusPanel(CwAPI3D::ControllerFactory* factory)
    {
      auto logger = PluginUiSession::instance().logger();
      if (adapters::driving::shouldSkipStatisticsUi(QCoreApplication::instance()))
      {
        if (logger)
        {
          logger->warn("Skipping statistics UI; no QCoreApplication");
        }
        return nullptr;
      }

      if (factory == nullptr)
      {
        if (logger)
        {
          logger->warn("Skipping statistics UI; null controller factory");
        }
        return nullptr;
      }

      ensureGraph(factory, logger);
      if (!gGraph || !gGraph->viewModel)
      {
        return nullptr;
      }

      if (!gGraph->dock.isNull())
      {
        return gGraph->dock.data();
      }

      auto* utility = factory->getUtilityController();
      if (utility == nullptr)
      {
        if (logger)
        {
          logger->warn("Skipping statistics UI; no utility controller");
        }
        return nullptr;
      }

      auto* host = adapters::driving::findMainWindow(reinterpret_cast<WId>(utility->get3DHWND()));
      if (host == nullptr)
      {
        if (logger)
        {
          logger->warn("Skipping statistics UI; host QMainWindow not found");
        }
        return nullptr;
      }

      ensureHostAbsentQtRuntimeVisible();
      gGraph->dock = new adapters::driving::StatisticsDockWidget(
        *gGraph->viewModel,
        *host,
        gGraph->logger.get());
      return gGraph->dock.data();
    }
  } // namespace

  void registerStatisticsPanelFactory()
  {
    PluginUiSession::instance().setPanelFactory(&createOrFocusPanel);
  }

} // namespace cw_api3d::composition
