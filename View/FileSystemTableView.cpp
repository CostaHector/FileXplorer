#include "View/FileSystemTableView.h"
#include "Actions/RenameActions.h"
#include "Actions/RightClickMenuActions.h"
#include "View/ViewHelper.h"

#include <QHeaderView>
#include <QMouseEvent>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/ViewActions.h"

FileSystemTableView::FileSystemTableView(MyQFileSystemModel* fsmModel, QWidget* parent) //
 : CustomTableView("FILE_SYSTEM", parent) //
{
  BindMenu(m_fsMenu);
  setModel(fsmModel);

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

  addActions(g_viewActions()._VIEW_ACTIONS->actions());
  addActions(g_viewActions()._VIDEO_PLAYERS->actions());
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());

  addActions(g_fileBasicOperationsActions().NEW->actions());
  addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
  addActions(g_fileBasicOperationsActions().FOLDER_MERGE->actions());
  addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
  addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());

  addActions(g_renameAg().RENAME_RIBBONS->actions());

  addAction(g_fileBasicOperationsActions().COPY_RECORDS);
  addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());

  addAction(g_rightClickActions()._SEARCH_IN_NET_EXPLORER);
}

void FileSystemTableView::dropEvent(QDropEvent* event) {
  View::dropEventCore(this, event);
}

void FileSystemTableView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, event);
}

void FileSystemTableView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, event);
}

void FileSystemTableView::dragLeaveEvent(QDragLeaveEvent* event) {
  View::dragLeaveEventCore(this, event);
}

auto FileSystemTableView::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier and e->key() == Qt::Key_Delete) {
    emit g_fileBasicOperationsActions().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QTableView::keyPressEvent(e);
}

void FileSystemTableView::mousePressEvent(QMouseEvent* event) {
  if (View::onMouseSidekeyBackwardForward(event->button())) {
    return;
  }
  return QTableView::mousePressEvent(event);
}

void FileSystemTableView::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::MouseButton::LeftButton) {
    View::mouseMoveEventCore(this, event);
    return;
  }
  return QTableView::mouseMoveEvent(event);
}
