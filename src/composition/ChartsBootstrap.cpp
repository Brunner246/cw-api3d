#include "src/composition/ChartsBootstrap.h"

#include "src/composition/Bootstrapping.h"
#include "src/composition/PluginUiSession.h"
#include "src/ports/Logger.h"
#ifdef CW_API3D_HAS_DRIVING
#include "src/composition/StatisticsPanelWiring.h"
#endif

#include <exception>

namespace cw_api3d::composition
{

  bool bootstrapChartsPlugin(CwAPI3D::ControllerFactory* factory) noexcept
  {
    ports::LoggerPtr logger;
    try
    {
#ifdef CW_API3D_HAS_DRIVING
      registerStatisticsPanelFactory();
#endif

      const auto bootstrapper = PluginBootstrapper::createProduction(factory);
      if (!bootstrapper)
      {
        return false;
      }

      logger = bootstrapper->logger();
      const auto result = bootstrapper->run();
      PluginUiSession::instance().setLogger(logger);
      PluginUiSession::instance().showOrFocus(factory);
      return result.has_value();
    }
    catch (const std::exception& ex)
    {
      if (logger)
      {
        logger->errorf("Unhandled exception during charts plugin bootstrap: {}", ex.what());
      }
      return false;
    }
    catch (...)
    {
      if (logger)
      {
        logger->error("Unknown unhandled exception during charts plugin bootstrap");
      }
      return false;
    }
  }

} // namespace cw_api3d::composition
