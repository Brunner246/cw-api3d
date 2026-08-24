#include "src/composition/PluginUiSession.h"

#include <utility>

namespace cw_api3d::composition
{
  namespace
  {
    std::unique_ptr<PluginUiSession>& sessionSlot()
    {
      static std::unique_ptr<PluginUiSession> instance;
      return instance;
    }
  } // namespace

  PluginUiSession::PluginUiSession(IStatisticsPanel& panel) noexcept
    : mPanel(&panel)
  {
  }

  PluginUiSession& PluginUiSession::instance()
  {
    auto& slot = sessionSlot();
    if (!slot)
    {
      slot = std::make_unique<PluginUiSession>();
    }
    return *slot;
  }

  void PluginUiSession::resetForTest(IStatisticsPanel& panel)
  {
    sessionSlot() = std::make_unique<PluginUiSession>(panel);
  }

  void PluginUiSession::resetInstance() noexcept
  {
    sessionSlot().reset();
  }

  void PluginUiSession::setLogger(ports::interfaces::LoggerPtr logger) noexcept
  {
    mLogger = std::move(logger);
  }

  ports::interfaces::LoggerPtr PluginUiSession::logger() const noexcept
  {
    return mLogger;
  }

  void PluginUiSession::setPanelFactory(PanelFactory factory)
  {
    mFactory = std::move(factory);
  }

  void PluginUiSession::showOrFocus()
  {
    showOrFocus(nullptr);
  }

  void PluginUiSession::showOrFocus(CwAPI3D::ControllerFactory* factory)
  {
    if (mFactory)
    {
      mPanel = mFactory(factory);
      if (mPanel == nullptr)
      {
        if (mLogger)
        {
          mLogger->warn("Skipping statistics UI");
        }
        return;
      }
      mPanel->showOrFocus();
      return;
    }

    if (mPanel != nullptr)
    {
      mPanel->showOrFocus();
      return;
    }

    if (mLogger)
    {
      mLogger->warn("Skipping statistics UI; no panel factory registered");
    }
  }

} // namespace cw_api3d::composition
