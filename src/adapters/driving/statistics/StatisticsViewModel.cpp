#include "src/adapters/driving/statistics/StatisticsViewModel.h"

#include <QTimer>

#include <cmath>
#include <limits>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace cw_api3d::adapters::driving
{

  namespace
  {

    constexpr int enabledChartTransitionMs = 400;

  }

  StatisticsViewModel::StatisticsViewModel(
    application::IFetchElementSnapshotUseCase& fetchUseCase,
    application::IActivateElementsUseCase& activateUseCase,
    std::function<bool()> clientAreaAnimationEnabled,
    QObject* parent)
    : QObject(parent)
    , mFetchUseCase(&fetchUseCase)
    , mActivateUseCase(&activateUseCase)
    , mBuckets(this)
    , mLocale(QLocale::system())
  {
    const bool osAnimations = clientAreaAnimationEnabled ? clientAreaAnimationEnabled() : queryClientAreaAnimation();
    mAnimationsEnabled = osAnimations;
  }

  StatisticsViewModel::Universe StatisticsViewModel::universe() const noexcept
  {
    return mUniverse;
  }

  void StatisticsViewModel::setUniverse(const Universe universe)
  {
    if (mUniverse == universe)
    {
      return;
    }
    mUniverse = universe;
    emit universeChanged();
  }

  StatisticsViewModel::Axis StatisticsViewModel::axis() const noexcept
  {
    return mAxis;
  }

  void StatisticsViewModel::setAxis(const Axis axis)
  {
    if (mAxis == axis)
    {
      return;
    }
    mAxis = axis;
    emit axisChanged();
    reaggregate();
  }

  StatisticsViewModel::ChartKind StatisticsViewModel::chartKind() const noexcept
  {
    return mChartKind;
  }

  void StatisticsViewModel::setChartKind(const ChartKind chartKind)
  {
    if (mChartKind == chartKind)
    {
      return;
    }
    mChartKind = chartKind;
    emit chartKindChanged();
  }

  bool StatisticsViewModel::busy() const noexcept
  {
    return mBusy;
  }

  bool StatisticsViewModel::empty() const noexcept
  {
    return mEmpty;
  }

  QString StatisticsViewModel::errorMessage() const
  {
    return mErrorMessage;
  }

  bool StatisticsViewModel::chartAvailable() const noexcept
  {
    return mChartAvailable;
  }

  void StatisticsViewModel::setChartAvailable(const bool chartAvailable)
  {
    if (mChartAvailable == chartAvailable)
    {
      return;
    }
    mChartAvailable = chartAvailable;
    emit chartAvailableChanged();
  }

  BucketListModel* StatisticsViewModel::buckets() noexcept
  {
    return &mBuckets;
  }

  const BucketListModel* StatisticsViewModel::buckets() const noexcept
  {
    return &mBuckets;
  }

  int StatisticsViewModel::totalCount() const noexcept
  {
    return mTotalCount;
  }

  bool StatisticsViewModel::animationsEnabled() const noexcept
  {
    return mAnimationsEnabled;
  }

  int StatisticsViewModel::chartTransitionDuration() const noexcept
  {
    return mAnimationsEnabled ? enabledChartTransitionMs : 0;
  }

  QString StatisticsViewModel::volumeDisclaimer() const
  {
    return tr("rough volume (host does not subtract drillings/end-types)");
  }

  void StatisticsViewModel::setLocale(const QLocale& locale)
  {
    mLocale = locale;
    reaggregate();
  }

  void StatisticsViewModel::fetch()
  {
    if (mBusy || mFetchUseCase == nullptr)
    {
      return;
    }
    mBusy = true;
    emit busyChanged();
    QTimer::singleShot(0, this, &StatisticsViewModel::runFetch);
  }

  void StatisticsViewModel::selectBucket(const int index)
  {
    if (mActivateUseCase == nullptr)
    {
      return;
    }
    if (index < 0 || index >= mBuckets.rowCount())
    {
      return;
    }

    const auto ids = mBuckets.memberIdsAt(index);
    const auto result = mActivateUseCase->execute(ids);
    if (!result.has_value())
    {
      mErrorMessage = QString::fromStdString(result.error());
      emit errorMessageChanged();
    }
  }

  void StatisticsViewModel::runFetch()
  {
    auto result = mFetchUseCase->execute(toApplication(mUniverse));
    if (!result.has_value())
    {
      mErrorMessage = QString::fromStdString(result.error());
      mEmpty = false;
      emit errorMessageChanged();
      emit emptyChanged();
      mBusy = false;
      emit busyChanged();
      return;
    }

    mSnapshot = std::move(*result);
    mHasSnapshot = true;
    if (!mErrorMessage.isEmpty())
    {
      mErrorMessage.clear();
      emit errorMessageChanged();
    }
    reaggregate();
    mBusy = false;
    emit busyChanged();
  }

  void StatisticsViewModel::reaggregate()
  {
    if (!mHasSnapshot)
    {
      return;
    }

    const auto series = mAggregator.aggregate(mSnapshot, toApplication(mAxis));
    QVector<QString> labels;
    labels.reserve(static_cast<int>(series.buckets.size()));
    for (const auto& bucket : series.buckets)
    {
      labels.push_back(displayLabelFor(bucket));
    }

    mTotalCount = static_cast<int>(mSnapshot.records.size());
    mEmpty = mSnapshot.records.empty();
    mBuckets.setBuckets(series.buckets, static_cast<std::size_t>(mTotalCount), std::move(labels));
    emit totalCountChanged();
    emit emptyChanged();
  }

  QString StatisticsViewModel::displayLabelFor(const application::StatisticBucket& bucket) const
  {
    if (isDimensionAxis() && bucket.lowerEdge.has_value() && bucket.upperEdge.has_value())
    {
      return formatDimensionLabel(bucket);
    }
    if (bucket.label.empty())
    {
      return tr("Empty");
    }
    return QString::fromStdString(bucket.label);
  }

  QString StatisticsViewModel::formatDimensionLabel(const application::StatisticBucket& bucket) const
  {
    return formatNumber(*bucket.lowerEdge) + QStringLiteral("–") + formatNumber(*bucket.upperEdge) + QChar(' ') + unitSuffix();
  }

  QString StatisticsViewModel::formatNumber(const double value) const
  {
    if (std::isfinite(value) && value == std::floor(value)
        && std::abs(value) <= static_cast<double>(std::numeric_limits<qlonglong>::max()))
    {
      return mLocale.toString(static_cast<qlonglong>(value));
    }
    return mLocale.toString(value, 'f', 3);
  }

  QString StatisticsViewModel::unitSuffix() const
  {
    switch (mAxis)
    {
      case Axis::Length:
      case Axis::Width:
      case Axis::Height:
        return tr("mm");
      case Axis::Volume:
        return tr("mm³");
      default:
        return {};
    }
  }

  bool StatisticsViewModel::isDimensionAxis() const noexcept
  {
    switch (mAxis)
    {
      case Axis::Length:
      case Axis::Width:
      case Axis::Height:
      case Axis::Volume:
        return true;
      default:
        return false;
    }
  }

  application::ElementUniverse StatisticsViewModel::toApplication(const Universe universe) noexcept
  {
    switch (universe)
    {
      case Universe::All:
        return application::ElementUniverse::All;
      case Universe::Active:
        return application::ElementUniverse::Active;
    }
    return application::ElementUniverse::Active;
  }

  application::StatisticAxis StatisticsViewModel::toApplication(const Axis axis) noexcept
  {
    switch (axis)
    {
      case Axis::Type:
        return application::StatisticAxis::Type;
      case Axis::Material:
        return application::StatisticAxis::Material;
      case Axis::Name:
        return application::StatisticAxis::Name;
      case Axis::Length:
        return application::StatisticAxis::Length;
      case Axis::Width:
        return application::StatisticAxis::Width;
      case Axis::Height:
        return application::StatisticAxis::Height;
      case Axis::Volume:
        return application::StatisticAxis::Volume;
    }
    return application::StatisticAxis::Type;
  }

  bool StatisticsViewModel::queryClientAreaAnimation()
  {
#ifdef _WIN32
    BOOL enabled = TRUE;
    if (SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &enabled, 0) == FALSE)
    {
      return true;
    }
    return enabled != FALSE;
#else
    return true;
#endif
  }

} // namespace cw_api3d::adapters::driving
