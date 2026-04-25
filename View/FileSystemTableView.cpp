#include "FileSystemTableView.h"
#include "FileOpActs.h"
#include "RenameActions.h"
#include "RightClickMenuActions.h"
#include "RightClickMenu.h"

#include "ViewActions.h"
#include "ViewHelper.h"
#include "RateHelper.h"
#include "NotificatorMacro.h"
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>
#include <QMouseEvent>

FileSystemTableView::FileSystemTableView(FileSystemModel* fsmModel, QWidget* parent) //
  : CustomTableView{"FILE_SYSTEM", parent}
  , _fsModel{fsmModel} { //
  CHECK_NULLPTR_RETURN_VOID(_fsModel);
  setProperty("showBackgroundImageFlag", true);
  PushFrontExclusiveActions(GetRightClickMenuActions(this));
  setModel(_fsModel);

  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  subscribe();

  InitTableView();
}

void FileSystemTableView::subscribe() {
  addAction(g_rightClickActions()._CALC_MD5_ACT);
  addAction(g_rightClickActions()._PROPERTIES);

  addAction(g_viewActions()._NAVIGATION_PANE);
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

void FileSystemTableView::initExclusivePreferenceSetting() {
  CustomTableView::m_defaultShowBackgroundImage = true;
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

void FileSystemTableView::keyPressEvent(QKeyEvent* e) {
  CHECK_NULLPTR_RETURN_VOID(e);
  const int ky = e->key();
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier && ky == Qt::Key_Delete) {
    emit FileOpActs::GetInst().MOVE_TO_TRASHBIN->triggered();
    e->accept();
    return;
  }
  QTableView::keyPressEvent(e);
}
