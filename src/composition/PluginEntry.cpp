#include "src/composition/Bootstrapping.h"
#ifdef CW_API3D_HAS_DRIVING
#include "src/composition/StatisticsPanelWiring.h"
#endif

#ifndef CWAPI3D_PLUGIN_NAME
#define CWAPI3D_PLUGIN_NAME L"cw_api3d"
#endif

#ifndef CWAPI3D_AUTHOR_NAME
#define CWAPI3D_AUTHOR_NAME L"cadwork"
#endif

#ifndef CWAPI3D_AUTHOR_EMAIL
#define CWAPI3D_AUTHOR_EMAIL L"cwapi3d@cadwork.ch"
#endif

#include <cwapi3d/CwAPI3D.h>

/// In cadwork CwAPI3D conventions, plugin_x64_init returning false indicates
/// the plugin stays loaded / completes cleanly without triggering an interactive
/// '<Return> to continue' host modal.
CWAPI3D_PLUGIN bool plugin_x64_init(CwAPI3D::ControllerFactory* aFactory)
{
  try
  {
#ifdef CW_API3D_HAS_DRIVING
    cw_api3d::composition::registerStatisticsPanelFactory();
#endif
    [[maybe_unused]] const bool success = cw_api3d::composition::bootstrapPlugin(aFactory);
  }
  catch (...)
  {
  }
  return false;
}

CWAPI3D_PLUGIN bool init_cwapi3d(CwAPI3D::ControllerFactory* aFactory)
{
  try
  {
#ifdef CW_API3D_HAS_DRIVING
    cw_api3d::composition::registerStatisticsPanelFactory();
#endif
    [[maybe_unused]] const bool success = cw_api3d::composition::bootstrapPlugin(aFactory);
  }
  catch (...)
  {
  }
  return false;
}
