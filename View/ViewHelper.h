#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QFileSystemModel>
#include <QTableView>

#include <QListView>
#include <QPushButton>

#include "Actions/AddressBarActions.h"
#include "MyQFileSystemModel.h"
#include "PublicVariable.h"

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

  static void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order) {
    PreferenceSettings().setValue(MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, logicalIndex);
    PreferenceSettings().setValue(MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.name, HEADERVIEW_SORT_INDICATOR_ORDER::SortOrderEnum2String(order));

    qDebug("sort logicalIndex [%d]",
           PreferenceSettings()
               .value(MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.v)
               .toInt());
    qDebug("order [%s]", PreferenceSettings()
                             .value(MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.name, MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.v)
                             .toString()
                             .toStdString()
                             .c_str());
  }

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

  static void UpdateItemViewFontSizeCore(QAbstractItemView* view) {
    if (view == nullptr) {
      qDebug("UpdateItemViewFontSizeCore view* pointer is nullptr");
      return;
    }
    const auto fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
    QFont defaultFont(view->font());
    defaultFont.setPointSize(fontSize);
    view->setFont(defaultFont);
  }

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
