#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QFileSystemModel>
#include <QTableView>

#include <QListView>
#include <QPushButton>

#include "MyQFileSystemModel.h"
#include "PublicVariable.h"

#include <QDrag>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QPainter>
#include <QToolTip>

constexpr int ROW_SECTION_HEIGHT = 10;
const QPoint TOOLTIP_MSG_PNG_DEV(48, 0);

const QString TOOLTIP_MSG_LINK("<font color=\"blue\">LINK---------------------------------------------------: <br/>%1</font>");
const QString TOOLTIP_MSG_CP("<font color=\"red\">CP---------------------------------------------------: <br/>%1</font>");
const QString TOOLTIP_MSG_MV("<font color=\"green\">MV---------------------------------------------------: <br/>%1</font>");

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

  static auto onMouseSidekeyBackwardForward(Qt::MouseButton mousebutton, QPushButton* backwardBtn, QPushButton* forwardBtn) -> bool {
    switch (mousebutton) {
      case Qt::MouseButton::BackButton:
        if (backwardBtn) {
          emit backwardBtn->clicked();
        }
        return true;
      case Qt::MouseButton::ForwardButton:
        if (forwardBtn) {
          emit forwardBtn->clicked();
        }
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

  static QModelIndexList selectedIndexes(QAbstractItemView* view) {  // ignore other column, keep the first column
    if (qobject_cast<QListView*>(view) != nullptr) {                 // TODO check here
      return view->selectionModel()->selectedIndexes();
    }
    return view->selectionModel()->selectedRows();
  }

  static void changeDropAction(QDropEvent* event, QPoint pnt=QPoint(), const QString& name="", QWidget* w=nullptr);

  static void dropEventCore(QAbstractItemView* view, QDropEvent* event);

  static void dragEnterEventCore(QAbstractItemView* view, QDragEnterEvent* event);

  static void dragMoveEventCore(QAbstractItemView* view, QDragMoveEvent* event);

  static void dragLeaveEventCore(QAbstractItemView* view, QDragLeaveEvent* event);

  static QPixmap PaintDraggedFilesFolders(const QString& firstSelectedAbsPath, const int selectedCnt);

  static void mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event) {
    if (event->buttons() != Qt::MouseButton::LeftButton) {
      return;
    }
    const QModelIndexList& mixed = View::selectedIndexes(view);
    if (mixed.isEmpty()) {
      event->ignore();
      return;
    }

    auto* _model = dynamic_cast<MyQFileSystemModel*>(view->model());
    if (_model == nullptr) {
      qDebug("[mouseMove] _model is nullptr");
      return;
    }
    if (_model->rootPath().isEmpty()) {
      qDebug("Ignore. Disk move is disabled[C:/,D:/,E:/]");
      return;
    }

    QList<QUrl> urls;
    urls.reserve(mixed.size());
    for (const auto& ind : mixed) {
      urls.append(QUrl::fromLocalFile(_model->filePath(ind)));
    }

    QMimeData* mime = new QMimeData;
    mime->setUrls(urls);
    QDrag drag(view);
    drag.setMimeData(mime);

    const QPixmap dragPixmap = View::PaintDraggedFilesFolders(urls[0].toLocalFile(), mixed.size());
    drag.setPixmap(dragPixmap);
    drag.exec(Qt::DropAction::LinkAction | Qt::DropAction::CopyAction | Qt::DropAction::MoveAction);
  }
};

#endif  // VIEWHELPER_H
