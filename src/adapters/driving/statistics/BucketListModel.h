#pragma once

#include "src/application/ElementSnapshot.h"
#include "src/application/StatisticSeries.h"

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>
#include <cstddef>
#include <vector>

namespace cw_api3d::adapters::driving
{

  class BucketListModel final : public QAbstractListModel
  {
    Q_OBJECT

  public:
    enum Role
    {
      LabelRole = Qt::UserRole + 1,
      CountRole,
      ShareRole
    };

    explicit BucketListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void setBuckets(
      std::vector<application::StatisticBucket> buckets,
      std::size_t totalCount,
      QVector<QString> displayLabels);

    [[nodiscard]] std::vector<application::ElementId> memberIdsAt(int row) const;
    [[nodiscard]] Q_INVOKABLE QString labelAt(int row) const;
    [[nodiscard]] Q_INVOKABLE int countAt(int row) const;

  private:
    std::vector<application::StatisticBucket> mBuckets;
    QVector<QString> mDisplayLabels;
    std::size_t mTotalCount{0};
  };

} // namespace cw_api3d::adapters::driving
