#include "CadworkUtilityAdapter.h"

namespace cw_api3d::adapters::driven::cadwork
{

  // Compile-time guards that the real host interface still fits the narrowed contract.
  // A CwAPI3D release that renames getPluginPath()/narrowData() fails here, not in the tests.
  static_assert(concepts::PluginPathSource<CwAPI3D::Interfaces::ICwAPI3DUtilityController>,
                "ICwAPI3DUtilityController must satisfy concepts::PluginPathSource");
  static_assert(ports::concepts::UtilityProvider<CadworkUtilityAdapter>,
                "CadworkUtilityAdapter must satisfy ports::concepts::UtilityProvider");

  template class UtilityControllerAdapter<CwAPI3D::Interfaces::ICwAPI3DUtilityController>;

} // namespace cw_api3d::adapters::driven::cadwork
