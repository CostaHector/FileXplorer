﻿#include "View/FileSystemTableView.h"
#include "View/ViewHelper.h"
#include "Actions/FolderPreviewActions.h"
#include "Actions/RenameActions.h"
#include "Actions/RightClickMenuActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/ViewActions.h"

#include <QHeaderView>
#include <QMouseEvent>

FileSystemTableView::FileSystemTableView(MyQFileSystemModel* fsmModel, QWidget* parent)  //
    : CustomTableView{"FILE_SYSTEM", parent}                                             //
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
  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
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

  addAction(g_folderPreviewActions().LISTS);
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
  if (event->button() & Qt::LeftButton) {
    mDragStartPosition = event->pos();
  }
  return QTableView::mousePressEvent(event);
}

void FileSystemTableView::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::MouseButton::LeftButton) {
    // ‌To prevent accidental drag operations that move files/folders, a drag threshold is added.
    // The drag event triggers only when the drag distance exceeds [specified number] pixels.
    if ((event->pos() - mDragStartPosition).manhattanLength() < View::START_DRAG_DIST) {
      return;
    }
    View::mouseMoveEventCore(this, event);
    return;
  }
  return QTableView::mouseMoveEvent(event);
}
