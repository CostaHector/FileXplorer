#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QFileSystemModel>
#include <QListView>
#include <QDragMoveEvent>
#include <QMimeData>

class View {
 public:
  virtual void subscribe() = 0;

  virtual void InitViewSettings() = 0;

  virtual void UpdateItemViewFontSize() = 0;

  static bool onMouseSidekeyBackwardForward(Qt::MouseButton mousebutton);

  static void UpdateItemViewFontSizeCore(QAbstractItemView* view);

  static inline QModelIndexList selectedIndexes(QAbstractItemView* view) {  // ignore other column, keep the first column
    if (view == nullptr) {
      return {};
    }
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
  static constexpr int START_DRAG_DIST{32};
  static constexpr int START_DRAG_DIST_MIN{10};
};

#endif  // VIEWHELPER_H
