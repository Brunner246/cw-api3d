#include "src/adapters/driving/statistics/BucketListModel.h"
#include "src/adapters/driving/statistics/CategoricalPalette.h"

namespace cw_api3d::adapters::driving
{

  BucketListModel::BucketListModel(QObject* parent)
    : QAbstractListModel(parent)
  {
  }

  int BucketListModel::rowCount(const QModelIndex& parent) const
  {
    if (parent.isValid())
    {
      return 0;
    }
    return static_cast<int>(mBuckets.size());
  }

  QVariant BucketListModel::data(const QModelIndex& index, const int role) const
  {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
    {
      return {};
    }

    const auto row = static_cast<std::size_t>(index.row());
    const auto& bucket = mBuckets[row];

    switch (role)
    {
      case Qt::DisplayRole:
      case LabelRole:
        if (row < static_cast<std::size_t>(mDisplayLabels.size()))
        {
          return mDisplayLabels[index.row()];
        }
        return QString::fromStdString(bucket.label);
      case CountRole:
        return static_cast<int>(bucket.count);
      case ShareRole:
        if (mTotalCount == 0)
        {
          return 0.0;
        }
        return static_cast<double>(bucket.count) / static_cast<double>(mTotalCount);
      case ColorRole:
        return CategoricalPalette::colorAt(row);
      default:
        return {};
    }
  }

  QHash<int, QByteArray> BucketListModel::roleNames() const
  {
    return {
      {LabelRole, QByteArrayLiteral("label")},
      {CountRole, QByteArrayLiteral("count")},
      {ShareRole, QByteArrayLiteral("share")},
      {ColorRole, QByteArrayLiteral("color")},
    };
  }

  void BucketListModel::setBuckets(
    std::vector<application::StatisticBucket> buckets,
    const std::size_t totalCount,
    QVector<QString> displayLabels)
  {
    beginResetModel();
    mBuckets = std::move(buckets);
    mTotalCount = totalCount;
    mDisplayLabels = std::move(displayLabels);
    endResetModel();
  }

  std::vector<application::ElementId> BucketListModel::memberIdsAt(const int row) const
  {
    if (row < 0 || row >= rowCount())
    {
      return {};
    }
    return mBuckets[static_cast<std::size_t>(row)].memberIds;
  }

  QString BucketListModel::labelAt(const int row) const
  {
    return data(index(row, 0), LabelRole).toString();
  }

  int BucketListModel::countAt(const int row) const
  {
    return data(index(row, 0), CountRole).toInt();
  }

  QColor BucketListModel::colorAt(const int row) const
  {
    return data(index(row, 0), ColorRole).value<QColor>();
  }

} // namespace cw_api3d::adapters::driving
