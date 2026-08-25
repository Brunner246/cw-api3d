#pragma once

#include "src/adapters/driving/statistics/BucketListModel.h"
#include "src/application/ActivateElementsUseCase.h"
#include "src/application/ElementSnapshot.h"
#include "src/application/ElementStatisticsAggregator.h"
#include "src/application/FetchElementSnapshotUseCase.h"

#include <QLocale>
#include <QObject>
#include <QString>
#include <functional>

namespace cw_api3d::adapters::driving
{

  class StatisticsViewModel final : public QObject
  {
    Q_OBJECT
    Q_PROPERTY(Universe universe READ universe WRITE setUniverse NOTIFY universeChanged)
    Q_PROPERTY(Axis axis READ axis WRITE setAxis NOTIFY axisChanged)
    Q_PROPERTY(ChartKind chartKind READ chartKind WRITE setChartKind NOTIFY chartKindChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool chartAvailable READ chartAvailable WRITE setChartAvailable NOTIFY chartAvailableChanged)
    Q_PROPERTY(BucketListModel* buckets READ buckets CONSTANT)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled CONSTANT)
    Q_PROPERTY(int chartTransitionDuration READ chartTransitionDuration CONSTANT)
    Q_PROPERTY(QString volumeDisclaimer READ volumeDisclaimer CONSTANT)

  public:
    enum class Universe
    {
      Active,
      All
    };
    Q_ENUM(Universe)

    enum class Axis
    {
      Type,
      Material,
      Name,
      Length,
      Width,
      Height,
      Volume
    };
    Q_ENUM(Axis)

    enum class ChartKind
    {
      Bar,
      Pie
    };
    Q_ENUM(ChartKind)

    explicit StatisticsViewModel(
      application::IFetchElementSnapshotUseCase& fetchUseCase,
      application::IActivateElementsUseCase& activateUseCase,
      std::function<bool()> clientAreaAnimationEnabled = {},
      QObject* parent = nullptr);

    [[nodiscard]] Universe universe() const noexcept;
    void setUniverse(Universe universe);

    [[nodiscard]] Axis axis() const noexcept;
    void setAxis(Axis axis);

    [[nodiscard]] ChartKind chartKind() const noexcept;
    void setChartKind(ChartKind chartKind);

    [[nodiscard]] bool busy() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] QString errorMessage() const;
    [[nodiscard]] bool chartAvailable() const noexcept;
    void setChartAvailable(bool chartAvailable);
    [[nodiscard]] BucketListModel* buckets() noexcept;
    [[nodiscard]] const BucketListModel* buckets() const noexcept;
    [[nodiscard]] int totalCount() const noexcept;
    [[nodiscard]] bool animationsEnabled() const noexcept;
    [[nodiscard]] int chartTransitionDuration() const noexcept;
    [[nodiscard]] QString volumeDisclaimer() const;

    void setLocale(const QLocale& locale);

    Q_INVOKABLE void fetch();
    Q_INVOKABLE void selectBucket(int index);

  signals:
    void universeChanged();
    void axisChanged();
    void chartKindChanged();
    void busyChanged();
    void emptyChanged();
    void errorMessageChanged();
    void chartAvailableChanged();
    void totalCountChanged();

  private:
    void runFetch();
    void reaggregate();
    [[nodiscard]] QString displayLabelFor(const application::StatisticBucket& bucket) const;
    [[nodiscard]] QString formatDimensionLabel(const application::StatisticBucket& bucket) const;
    [[nodiscard]] QString formatNumber(double value) const;
    [[nodiscard]] QString unitSuffix() const;
    [[nodiscard]] bool isDimensionAxis() const noexcept;
    [[nodiscard]] static application::ElementUniverse toApplication(Universe universe) noexcept;
    [[nodiscard]] static application::StatisticAxis toApplication(Axis axis) noexcept;
    [[nodiscard]] static bool queryClientAreaAnimation();

    application::IFetchElementSnapshotUseCase* mFetchUseCase{nullptr};
    application::IActivateElementsUseCase* mActivateUseCase{nullptr};
    application::ElementStatisticsAggregator mAggregator;
    application::ElementSnapshot mSnapshot;
    bool mHasSnapshot{false};
    BucketListModel mBuckets;
    Universe mUniverse{Universe::Active};
    Axis mAxis{Axis::Type};
    ChartKind mChartKind{ChartKind::Bar};
    bool mBusy{false};
    bool mEmpty{false};
    QString mErrorMessage;
    bool mChartAvailable{true};
    int mTotalCount{0};
    bool mAnimationsEnabled{true};
    QLocale mLocale;
  };

} // namespace cw_api3d::adapters::driving
