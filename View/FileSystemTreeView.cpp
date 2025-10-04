#include "FileSystemTreeView.h"
#include "ViewHelper.h"
#include "StyleSheet.h"
#include "MemoryKey.h"

#include "RightClickMenuActions.h"
#include "FileOpActs.h"
#include "RenameActions.h"
#include "ViewActions.h"
#include <QHeaderView>
#include <QMouseEvent>

FileSystemTreeView::FileSystemTreeView(FileSystemModel* fsmModel, QWidget* parent)  //
    : QTreeView{parent}, _fsModel{fsmModel}                                         //
{
  CHECK_NULLPTR_RETURN_VOID(_fsModel);
  m_fsMenu = new (std::nothrow) RightClickMenu("Right click menu", this);
  CHECK_NULLPTR_RETURN_VOID(m_fsMenu);

  setModel(fsmModel);
  InitViewSettings();

  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  const int fontSize = Configuration().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  FileSystemTreeView::subscribe();
}

void FileSystemTreeView::subscribe() {
  connect(header(), &QHeaderView::sectionResized, this, [this]() { Configuration().setValue("FILE_EXPLORER_HEADER_GEOMETRY_TREE_VIEW", header()->saveState()); });

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

auto FileSystemTreeView::InitViewSettings() -> void {
  //  setShowGrid(false);
  setAlternatingRowColors(true);
  setSortingEnabled(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  if (Configuration().contains("FILE_EXPLORER_HEADER_GEOMETRY_TREE_VIEW")) {
    header()->restoreState(Configuration().value("FILE_EXPLORER_HEADER_GEOMETRY_TREE_VIEW").toByteArray());
  }
  sizeHintForRow(StyleSheet::ROW_SECTION_HEIGHT);
  FileSystemTreeView::UpdateItemViewFontSize();
}

auto FileSystemTreeView::UpdateItemViewFontSize() -> void {
  View::UpdateItemViewFontSizeCore(this);
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

auto FileSystemTreeView::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier and e->key() == Qt::Key_Delete) {
    emit FileOpActs::GetInst().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QTreeView::keyPressEvent(e);
}

void FileSystemTreeView::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  CHECK_NULLPTR_RETURN_VOID(m_fsMenu);
#ifndef RUNNING_UNIT_TESTS
  m_fsMenu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
#endif
}

void FileSystemTreeView::mousePressEvent(QMouseEvent* event) {
  if (View::onMouseSidekeyBackwardForward(event->modifiers(), event->button())) {
    event->accept();
    return;
  }
  if (event->button() == Qt::LeftButton) {
    mDragStartPosition = event->pos();
  }
  return QTreeView::mousePressEvent(event);
}

void FileSystemTreeView::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::MouseButton::LeftButton) {
    if ((event->pos() - mDragStartPosition).manhattanLength() < View::START_DRAG_DIST) {
      return;
    }
    View::mouseMoveEventCore(this, event);
    return;
  }
  return QTreeView::mouseMoveEvent(event);
}
