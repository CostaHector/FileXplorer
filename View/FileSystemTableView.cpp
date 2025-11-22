#include "FileSystemTableView.h"
#include "FileOpActs.h"
#include "FolderPreviewActions.h"
#include "RenameActions.h"
#include "RightClickMenuActions.h"
#include "ViewActions.h"
#include "ViewHelper.h"
#include "RateHelper.h"
#include "NotificatorMacro.h"
#include <QHeaderView>
#include <QMouseEvent>

FileSystemTableView::FileSystemTableView(FileSystemModel* fsmModel, QWidget* parent) //
  : CustomTableView{"FILE_SYSTEM", parent}
  , _fsModel{fsmModel} { //
  CHECK_NULLPTR_RETURN_VOID(_fsModel)
  BindMenu(new (std::nothrow) RightClickMenu("Right click menu", this));
  setModel(_fsModel);

  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  subscribe();

  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  InitTableView();
}

void FileSystemTableView::subscribe() {
  addAction(g_rightClickActions()._CALC_MD5_ACT);
  addAction(g_rightClickActions()._PROPERTIES);

  addAction(g_viewActions().NAVIGATION_PANE);
  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
  addActions(FileOpActs::GetInst().OPEN_AG->actions());

  addActions(FileOpActs::GetInst().NEW->actions());
  addActions(FileOpActs::GetInst().CUT_COPY_PASTE->actions());
  addActions(FileOpActs::GetInst().FOLDER_MERGE->actions());
  addActions(FileOpActs::GetInst().MOVE_COPY_TO->actions());
  addActions(FileOpActs::GetInst().UNDO_REDO_RIBBONS->actions());

  addActions(g_renameAg().RENAME_RIBBONS->actions());

  addAction(FileOpActs::GetInst().COPY_RECORDS);
  addActions(FileOpActs::GetInst().SELECTION_RIBBONS->actions());
  addActions(FileOpActs::GetInst().DELETE_ACTIONS->actions());

  addAction(g_rightClickActions()._SEARCH_IN_NET_EXPLORER);
}

void FileSystemTableView::dropEvent(QDropEvent* event) {
  View::dropEventCore(this, _fsModel, event);
}

void FileSystemTableView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, _fsModel, event);
}

void FileSystemTableView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, _fsModel, event);
}

void FileSystemTableView::dragLeaveEvent(QDragLeaveEvent* event) {
  View::dragLeaveEventCore(_fsModel, event);
}

auto FileSystemTableView::keyPressEvent(QKeyEvent* e) -> void {
  CHECK_NULLPTR_RETURN_VOID(e);
  const int ky = e->key();
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier && ky == Qt::Key_Delete) {
    emit FileOpActs::GetInst().MOVE_TO_TRASHBIN->triggered();
    e->accept();
    return;
  }
  QTableView::keyPressEvent(e);
}

void FileSystemTableView::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (event->button() == Qt::LeftButton) {
    mDragStartPosition = event->pos();
  }
  CustomTableView::mousePressEvent(event);
}

void FileSystemTableView::mouseMoveEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (event->buttons() == Qt::MouseButton::LeftButton) {
    // ‌To prevent accidental drag operations that move files/folders, a drag threshold is added.
    // The drag event triggers only when the drag distance exceeds [specified number] pixels.
    if ((event->pos() - mDragStartPosition).manhattanLength() < View::START_DRAG_DIST) {
      event->ignore();
      return;
    }
    View::mouseMoveEventCore(this, event);
    event->accept();
    return;
  }
  return QTableView::mouseMoveEvent(event);
}
