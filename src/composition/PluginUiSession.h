#pragma once

#include "src/composition/IStatisticsPanel.h"
#include "src/ports/Logger.h"

#include <functional>
#include <memory>

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

  class PluginUiSession
  {
  public:
    using PanelFactory = std::function<IStatisticsPanel*(CwAPI3D::ControllerFactory*)>;

    explicit PluginUiSession(IStatisticsPanel& panel) noexcept;
    PluginUiSession() noexcept = default;

    [[nodiscard]] static PluginUiSession& instance();
    static void resetForTest(IStatisticsPanel& panel);
    static void resetInstance() noexcept;

    void setLogger(ports::interfaces::LoggerPtr logger) noexcept;
    [[nodiscard]] ports::interfaces::LoggerPtr logger() const noexcept;
    void setPanelFactory(PanelFactory factory);

    void showOrFocus();
    void showOrFocus(CwAPI3D::ControllerFactory* factory);

  private:
    IStatisticsPanel* mPanel{nullptr};
    PanelFactory mFactory;
    ports::interfaces::LoggerPtr mLogger;
  };

} // namespace cw_api3d::composition
