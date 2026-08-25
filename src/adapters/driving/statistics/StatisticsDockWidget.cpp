#include "src/adapters/driving/statistics/StatisticsDockWidget.h"

#include "src/adapters/driving/statistics/BucketListModel.h"

#include <QAbstractTableModel>
#include <QCoreApplication>
#include <QHeaderView>
#include <QLabel>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

namespace cw_api3d::adapters::driving
{
  namespace
  {
    class BucketTableModel final : public QAbstractTableModel
    {
    public:
      explicit BucketTableModel(BucketListModel& source, QObject* parent = nullptr)
        : QAbstractTableModel(parent)
        , mSource(&source)
      {
        QObject::connect(&source, &QAbstractItemModel::modelReset, this, [this]() {
          beginResetModel();
          endResetModel();
        });
      }

      [[nodiscard]] int rowCount(const QModelIndex& parent) const override
      {
        return parent.isValid() ? 0 : mSource->rowCount();
      }

      [[nodiscard]] int columnCount(const QModelIndex& parent) const override
      {
        return parent.isValid() ? 0 : 2;
      }

      [[nodiscard]] QVariant data(const QModelIndex& index, const int role) const override
      {
        if (!index.isValid() || role != Qt::DisplayRole)
        {
          return {};
        }
        if (index.column() == 0)
        {
          return mSource->labelAt(index.row());
        }
        return mSource->countAt(index.row());
      }

      [[nodiscard]] QVariant headerData(
        const int section,
        const Qt::Orientation orientation,
        const int role) const override
      {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        {
          return {};
        }
        return section == 0 ? QObject::tr("Label") : QObject::tr("Count");
      }

    private:
      BucketListModel* mSource{nullptr};
    };

    void ensureFusionStyle()
    {
      if (QQuickStyle::name().isEmpty())
      {
        QQuickStyle::setStyle(QStringLiteral("Fusion"));
      }
    }
  } // namespace

  StatisticsDockWidget::StatisticsDockWidget(
    StatisticsViewModel& viewModel,
    QMainWindow& host,
    ports::interfaces::ILogger* logger)
    : QDockWidget(&host)
    , mViewModel(&viewModel)
    , mLogger(logger)
  {
    configureChrome();
    buildContent();
    host.addDockWidget(Qt::LeftDockWidgetArea, this);
    connectAboutToQuit();
  }

  void StatisticsDockWidget::showOrFocus()
  {
    show();
    raise();
    activateWindow();
  }

  StatisticsViewModel& StatisticsDockWidget::viewModel() noexcept
  {
    return *mViewModel;
  }

  const StatisticsViewModel& StatisticsDockWidget::viewModel() const noexcept
  {
    return *mViewModel;
  }

  bool StatisticsDockWidget::chartSurfaceReady() const noexcept
  {
    return mChartSurfaceReady;
  }

  void StatisticsDockWidget::configureChrome()
  {
    setObjectName(QStringLiteral("cw_api3d.statisticsDock"));
    setWindowTitle(tr("Model statistics"));
    setFeatures(DockWidgetClosable | DockWidgetMovable | DockWidgetFloatable);
    setAllowedAreas(Qt::AllDockWidgetAreas);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowModality(Qt::NonModal);
    setStyleSheet(QStringLiteral("QDockWidget::title { background: #d4d4d4; color: #000000; padding: 4px; }"));
  }

  void StatisticsDockWidget::buildContent()
  {
    ensureFusionStyle();

    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);

    mBanner = new QLabel(container);
    mBanner->setObjectName(QStringLiteral("chartUnavailableBanner"));
    mBanner->setWordWrap(true);
    mBanner->setVisible(false);
    layout->addWidget(mBanner);

    mQuickWidget = new QQuickWidget(container);
    mQuickWidget->setObjectName(QStringLiteral("chartTable"));
    mQuickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mQuickWidget->setFocusPolicy(Qt::StrongFocus);
    mQuickWidget->engine()->addImportPath(QStringLiteral("qrc:/qt/qml"));
    mQuickWidget->rootContext()->setContextProperty(QStringLiteral("viewModel"), mViewModel);
    mQuickWidget->setSource(QUrl(QStringLiteral("qrc:/qt/qml/CwApi3d/Statistics/StatisticsPanel.qml")));
    layout->addWidget(mQuickWidget, 1);

    mTableView = new QTableView(container);
    mTableView->setObjectName(QStringLiteral("chartTableFallback"));
    mTableView->setModel(new BucketTableModel(*mViewModel->buckets(), mTableView));
    mTableView->horizontalHeader()->setStretchLastSection(true);
    mTableView->setVisible(false);
    layout->addWidget(mTableView, 1);

    setWidget(container);

    QCoreApplication::processEvents();
    if (mQuickWidget->status() != QQuickWidget::Ready)
    {
      installTableFallback();
    }
    else
    {
      QObject::connect(
        mQuickWidget,
        &QQuickWidget::statusChanged,
        this,
        [this](const QQuickWidget::Status status) {
          if (status == QQuickWidget::Error)
          {
            installTableFallback();
          }
        });
    }
  }

  void StatisticsDockWidget::installTableFallback()
  {
    mChartSurfaceReady = false;
    mViewModel->setChartAvailable(false);
    if (mQuickWidget != nullptr)
    {
      mQuickWidget->setVisible(false);
    }
    if (mTableView != nullptr)
    {
      mTableView->setVisible(true);
    }
    if (mBanner != nullptr)
    {
      mBanner->setText(tr("Chart surface is unavailable; showing table"));
      mBanner->setVisible(true);
    }
    if (mLogger != nullptr)
    {
      mLogger->warn("QQuickWidget failed to become Ready; using table fallback");
    }
  }

  void StatisticsDockWidget::connectAboutToQuit()
  {
    if (auto* app = QCoreApplication::instance(); app != nullptr)
    {
      QObject::connect(app, &QCoreApplication::aboutToQuit, this, [this]() {
        disconnect();
      });
    }
  }

} // namespace cw_api3d::adapters::driving
