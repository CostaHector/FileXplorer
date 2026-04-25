#include "FileSystemTreeView.h"
#include "ViewHelper.h"
#include "StyleSheet.h"
#include "MemoryKey.h"
#include "Configuration.h"

#include "RightClickMenuActions.h"
#include "RightClickMenu.h"
#include "FileOpActs.h"
#include "RenameActions.h"
#include "ViewActions.h"
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QMouseEvent>

FileSystemTreeView::FileSystemTreeView(FileSystemModel* fsmModel, QWidget* parent)  //
    : CustomTreeView{"FILE_SYSTEM_TREE", parent}, _fsModel{fsmModel}                                         //
{
  CHECK_NULLPTR_RETURN_VOID(_fsModel);
  PushFrontExclusiveActions(GetRightClickMenuActions(this));
  setModel(fsmModel);

  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers

  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  InitTreeView();
  FileSystemTreeView::subscribe();
}

void FileSystemTreeView::subscribe() {
  addAction(g_rightClickActions()._CALC_MD5_ACT);
  addAction(g_rightClickActions()._PROPERTIES);

  addAction(g_viewActions()._NAVIGATION_PANE);
  addActions(FileOpActs::GetInst().OPEN_AG->actions());

  addActions(FileOpActs::GetInst().NEW->actions());
  addActions(FileOpActs::GetInst().CUT_COPY_PASTE->actions());
  addActions(FileOpActs::GetInst().FOLDER_MERGE->actions());
  addActions(FileOpActs::GetInst().MOVE_COPY_TO->actions());
  addActions(FileOpActs::GetInst().UNDO_REDO_RIBBONS->actions());

  addActions(g_renameAg().RENAME_RIBBONS->actions());

  addActions(FileOpActs::GetInst().SELECTION_RIBBONS->actions());
  addActions(FileOpActs::GetInst().DELETE_ACTIONS->actions());
  addAction(g_rightClickActions()._SEARCH_IN_NET_EXPLORER);
}

void FileSystemTreeView::dropEvent(QDropEvent* event) {
  View::dropEventCore(this, _fsModel, event);
}

void FileSystemTreeView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, _fsModel, event);
}

void FileSystemTreeView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, _fsModel, event);
}

void FileSystemTreeView::dragLeaveEvent(QDragLeaveEvent* event) {
  View::dragLeaveEventCore(_fsModel, event);
}

void FileSystemTreeView::keyPressEvent(QKeyEvent* e) {
  CHECK_NULLPTR_RETURN_VOID(e);
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier and e->key() == Qt::Key_Delete) {
    emit FileOpActs::GetInst().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QTreeView::keyPressEvent(e);
}
