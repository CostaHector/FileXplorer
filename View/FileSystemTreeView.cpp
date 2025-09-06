#include "FileSystemTreeView.h"
#include "ViewHelper.h"
#include "StyleSheet.h"
#include "MemoryKey.h"

#include "RightClickMenuActions.h"
#include "FileBasicOperationsActions.h"
#include "RenameActions.h"
#include "ViewActions.h"
#include <QHeaderView>
#include <QMouseEvent>

FileSystemTreeView::FileSystemTreeView(FileSystemModel* fsmModel, QWidget* parent)  //
    : QTreeView{parent}                                                                //
{
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

  addActions(g_viewActions()._NAVI_ACTIONS->actions());
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());

  addActions(g_fileBasicOperationsActions().NEW->actions());
  addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
  addActions(g_fileBasicOperationsActions().FOLDER_MERGE->actions());
  addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
  addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());

  addActions(g_renameAg().RENAME_RIBBONS->actions());

  addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
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
  View::dropEventCore(this, event);
}

void FileSystemTreeView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, event);
}

void FileSystemTreeView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, event);
}

void FileSystemTreeView::dragLeaveEvent(QDragLeaveEvent* event) {
  View::dragLeaveEventCore(this, event);
}

auto FileSystemTreeView::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier and e->key() == Qt::Key_Delete) {
    emit g_fileBasicOperationsActions().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QTreeView::keyPressEvent(e);
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
