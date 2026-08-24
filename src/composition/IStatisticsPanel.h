#pragma once

namespace cw_api3d::composition
{

  struct IStatisticsPanel
  {
    virtual ~IStatisticsPanel() = default;

    virtual void showOrFocus() = 0;
  };

} // namespace cw_api3d::composition
