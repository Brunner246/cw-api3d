#pragma once

#include "src/adapters/driving/statistics/StatisticsViewModel.h"
#include "src/composition/IStatisticsPanel.h"
#include "src/ports/Logger.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QPointer>

class QLabel;
class QQuickWidget;
class QTableView;

namespace cw_api3d::adapters::driving
{

  class StatisticsDockWidget final : public QDockWidget
    , public composition::IStatisticsPanel
  {
    Q_OBJECT

  public:
    StatisticsDockWidget(
      StatisticsViewModel& viewModel,
      QMainWindow& host,
      ports::ILogger* logger = nullptr);

    void showOrFocus() override;

    [[nodiscard]] StatisticsViewModel& viewModel() noexcept;
    [[nodiscard]] const StatisticsViewModel& viewModel() const noexcept;
    [[nodiscard]] bool chartSurfaceReady() const noexcept;

  private:
    void configureChrome();
    void buildContent();
    void installTableFallback();
    void connectAboutToQuit();

    StatisticsViewModel* mViewModel{nullptr};
    ports::ILogger* mLogger{nullptr};
    QQuickWidget* mQuickWidget{nullptr};
    QTableView* mTableView{nullptr};
    QLabel* mBanner{nullptr};
    bool mChartSurfaceReady{true};
  };

} // namespace cw_api3d::adapters::driving
