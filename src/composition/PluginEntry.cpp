#include "src/composition/Bootstrapping.h"

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

CWAPI3D_PLUGIN bool plugin_x64_init(CwAPI3D::ControllerFactory* aFactory)
{
  [[maybe_unused]] const bool success = cw_api3d::composition::bootstrapPlugin(aFactory);
  return true;
}