#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QFileSystemModel>
#include <QTableView>

#include <QListView>
#include <QPushButton>

#include "Actions/AddressBarActions.h"
#include "public/PublicVariable.h"

#include <QDrag>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QPainter>
#include <QToolTip>

class View {
 public:
  virtual void subscribe() = 0;

  virtual void InitViewSettings() = 0;

  virtual void UpdateItemViewFontSize() = 0;

  static auto onMouseSidekeyBackwardForward(Qt::MouseButton mousebutton) -> bool {
    switch (mousebutton) {
      case Qt::MouseButton::BackButton:
        g_addressBarActions()._BACK_TO->triggered();
        return true;
      case Qt::MouseButton::ForwardButton:
        g_addressBarActions()._FORWARD_TO->triggered();
        return true;
      default:
        return false;
    }
  }

  static void UpdateItemViewFontSizeCore(QAbstractItemView* view);

  static inline QModelIndexList selectedIndexes(QAbstractItemView* view) {  // ignore other column, keep the first column
    if (dynamic_cast<QListView*>(view) != nullptr) {
      return view->selectionModel()->selectedIndexes();
    }
    return view->selectionModel()->selectedRows();
  }

  static bool onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& to);

  static void changeDropAction(QDropEvent* event);

  static void dragEnterEventCore(QAbstractItemView* view, QDragEnterEvent* event);
  static void dragMoveEventCore(QAbstractItemView* view, QDragMoveEvent* event);
  static void dropEventCore(QAbstractItemView* view, QDropEvent* event);
  static void dragLeaveEventCore(QAbstractItemView* view, QDragLeaveEvent* event);

  static void mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event);

  static QPixmap PaintDraggedFilesFolders(const QString& firstSelectedAbsPath, const int selectedCnt);
};

#endif  // VIEWHELPER_H
