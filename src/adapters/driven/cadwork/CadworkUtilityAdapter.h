#pragma once

#include "src/adapters/driven/cadwork/UtilityControllerAdapter.h"

#include <cwapi3d/ICwAPI3DString.h>
#include <cwapi3d/ICwAPI3DUtilityController.h>

namespace cw_api3d::adapters::driven::cadwork
{

  /// @brief Production instantiation of UtilityControllerAdapter against the cadwork host interface.
  /// This header is the single point where the adapter layer names a CwAPI3D type; the translation
  /// logic itself lives in UtilityControllerAdapter.h and is SDK-free.
  using CadworkUtilityAdapter = UtilityControllerAdapter<CwAPI3D::Interfaces::ICwAPI3DUtilityController>;

  // Compile-time guard that the real host interface still fits the narrowed contract.
  // A CwAPI3D release that renames getPluginPath()/narrowData() fails here, not in the tests.
  static_assert(concepts::PluginPathSource<CwAPI3D::Interfaces::ICwAPI3DUtilityController>,
                "ICwAPI3DUtilityController must satisfy concepts::PluginPathSource");

} // namespace cw_api3d::adapters::driven::cadwork
