#pragma once

#include <QCoreApplication>
#include <QMainWindow>
#include <QWidget>

namespace cw_api3d::adapters::driving
{

  [[nodiscard]] inline QMainWindow* findMainWindow(QWidget* widget) noexcept
  {
    while (widget != nullptr)
    {
      if (auto* mainWindow = qobject_cast<QMainWindow*>(widget))
      {
        return mainWindow;
      }
      widget = widget->parentWidget();
    }
    return nullptr;
  }

  [[nodiscard]] inline QMainWindow* findMainWindow(const WId windowId) noexcept
  {
    return findMainWindow(QWidget::find(windowId));
  }

  [[nodiscard]] inline bool shouldSkipStatisticsUi(const QCoreApplication* instance) noexcept
  {
    return instance == nullptr;
  }

} // namespace cw_api3d::adapters::driving
