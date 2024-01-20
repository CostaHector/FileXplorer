#include "View/DragDropTableView.h"

#include <QHeaderView>
#include <QMouseEvent>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RenameActions.h"
#include "PublicVariable.h"

DragDropTableView::DragDropTableView(MyQFileSystemModel* fsmModel, QPushButton* mouseSideKeyBackwardBtn, QPushButton* mouseSideKeyForwardBtn)
    : QTableView(),
      View(),
      backwardBtn(mouseSideKeyBackwardBtn),
      forwardBtn(mouseSideKeyForwardBtn),
      menu(new RightClickMenu("Right click menu", this)) {
  setModel(fsmModel);
  InitViewSettings();

  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  DragDropTableView::subscribe();
}

void DragDropTableView::subscribe() {
  connect(horizontalHeader(), &QHeaderView::sectionResized, this,
          [this]() { PreferenceSettings().setValue("FILE_EXPLORER_HEADER_GEOMETRY", horizontalHeader()->saveState()); });

  connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &View::onSortIndicatorChanged);
  addActions(g_fileBasicOperationsActions().OPEN->actions());

  addActions(g_fileBasicOperationsActions().NEW->actions());
  addActions(g_fileBasicOperationsActions().CUT_COPY_MERGE_PASTE->actions());
  addActions(g_fileBasicOperationsActions().FOLDER_MERGE->actions());
  addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
  addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());

  addActions(g_renameAg().RENAME_RIBBONS->actions());

  addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());

  connect(this, &QTableView::customContextMenuRequested, this, &DragDropTableView::on_ShowContextMenu);
}

auto DragDropTableView::InitViewSettings() -> void {
  setShowGrid(false);
  setAlternatingRowColors(true);
  setSortingEnabled(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  verticalHeader()->setVisible(false);
  verticalHeader()->setDefaultSectionSize(ROW_SECTION_HEIGHT);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  horizontalHeader()->setStretchLastSection(false);
  horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);

  horizontalHeader()->restoreState(PreferenceSettings().value("FILE_EXPLORER_HEADER_GEOMETRY").toByteArray());
  DragDropTableView::UpdateItemViewFontSize();
}

auto DragDropTableView::UpdateItemViewFontSize() -> void {
  View::UpdateItemViewFontSizeCore(this);
}

void DragDropTableView::dropEvent(QDropEvent* event) {
  View::dropEventCore(this, event);
  return QTableView::dropEvent(event);
}

void DragDropTableView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, event);
  return QTableView::dragEnterEvent(event);
}

void DragDropTableView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, event);
}

void DragDropTableView::mouseMoveEvent(QMouseEvent* event) {
  View::mouseMoveEventCore(this, event);
}

void DragDropTableView::on_ShowContextMenu(const QPoint pnt) {
  menu->popup(this->mapToGlobal(pnt));  // or QCursor::pos()
}

void DragDropTableView::mousePressEvent(QMouseEvent* event) {
  if (View::onMouseSidekeyBackwardForward(event->button(), backwardBtn, forwardBtn)) {
    return;
  }
  return QTableView::mousePressEvent(event);
}
