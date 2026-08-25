#pragma once

#include "src/composition/IStatisticsPanel.h"

#include <stdexcept>
#include <string>

namespace cw_api3d::tests::doubles
{

  class FakeStatisticsPanel final : public composition::IStatisticsPanel
  {
  public:
    int createCount{1};
    int showCount{0};
    bool throwOnShow{false};
    std::string throwMessage{"session/UI construction failed"};

    void showOrFocus() override
    {
      if (throwOnShow)
      {
        throw std::runtime_error(throwMessage);
      }
      ++showCount;
    }
  };

} // namespace cw_api3d::tests::doubles
