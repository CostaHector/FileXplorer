#include "FileSystemListView.h"
#include "PublicMacro.h"
#include "FileOpActs.h"
#include "RenameActions.h"
#include "RightClickMenuActions.h"
#include "ViewActions.h"

#include "FileSystemListView.h"
#include "ViewHelper.h"

#include <QHeaderView>
#include <QMouseEvent>

FileSystemListView::FileSystemListView(FileSystemModel* fsmModel, QWidget* parent)  //
    : CustomListView{"FILE_SYSTEM_LIST", parent}, _fsModel{fsmModel}                                       //
{
  CHECK_NULLPTR_RETURN_VOID(_fsModel);
  m_fsMenu = new (std::nothrow) RightClickMenu("Right click menu", this);
  CHECK_NULLPTR_RETURN_VOID(m_fsMenu);

  BindMenu(m_fsMenu);
  setModel(fsmModel);

  setDragDropMode(QAbstractItemView::DragDrop);

  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  subscribe();
  setWrapping(true);
  setUniformItemSizes(true);
}

void FileSystemListView::subscribe() {
  addAction(g_rightClickActions()._CALC_MD5_ACT);
  addAction(g_rightClickActions()._PROPERTIES);

  addAction(g_viewActions().NAVIGATION_PANE);
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

void FileSystemListView::dropEvent(QDropEvent* event) {
  View::dropEventCore(this, _fsModel, event);
}

void FileSystemListView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, _fsModel, event);
}

void FileSystemListView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, _fsModel, event);
}

void FileSystemListView::dragLeaveEvent(QDragLeaveEvent* event) {
  View::dragLeaveEventCore(_fsModel, event);
}

auto FileSystemListView::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier && e->key() == Qt::Key_Delete) {
    emit FileOpActs::GetInst().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QListView::keyPressEvent(e);
}

void FileSystemListView::mousePressEvent(QMouseEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  if (event->button() == Qt::LeftButton) {
    mDragStartPosition = event->pos();
  }
  CustomListView::mousePressEvent(event);
}

void FileSystemListView::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::MouseButton::LeftButton) {
    if ((event->pos() - mDragStartPosition).manhattanLength() < View::START_DRAG_DIST) {
      event->ignore();
      return;
    }
    View::mouseMoveEventCore(this, event);
    event->accept();
    return;
  }
  return QListView::mouseMoveEvent(event);
}
