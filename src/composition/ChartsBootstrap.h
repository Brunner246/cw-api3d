#pragma once

namespace CwAPI3D
{
  namespace Interfaces
  {
    class ICwAPI3DControllerFactory;
  }
  using ControllerFactory = Interfaces::ICwAPI3DControllerFactory;
} // namespace CwAPI3D

namespace cw_api3d::composition
{

  /// C-safe charts composition: kit path query, then show-or-focus the statistics panel.
  /// Never lets exceptions escape to the host C ABI.
  [[nodiscard]] bool bootstrapChartsPlugin(CwAPI3D::ControllerFactory* factory) noexcept;

} // namespace cw_api3d::composition
