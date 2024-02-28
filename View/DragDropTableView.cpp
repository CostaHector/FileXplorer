#include "View/DragDropTableView.h"

#include <QHeaderView>
#include <QMouseEvent>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RenameActions.h"
#include "Actions/ViewActions.h"
#include "PublicVariable.h"

const QString TABLEVIEW_STYLESHEET = "QTableView {"\
    "    show-decoration-selected: 1;"\
    "}"\
    "QTableView::item:alternate {"\
    "}"\
    "QTableView::item:selected {"\
    "    border-bottom: 1px inherit #FFFFFF;"\
    "}"\
    "QTableView::item:selected:!active {"\
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EEEEEE, stop: 1 #999999);"\
    "    color: #000000;"\
    "}"\
    "QTableView::item:selected:active {"\
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #99D1FF, stop: 1 #99D1FF);"\
    "    color: #000000;"\
    "    border-top: 2px solid #CCEBFF;"\
    "    border-bottom: 2px solid #CCEBFF;"\
    "}"\
    "QTableView::item:hover {"\
    "    background: #CCEBFF;"\
    "}";

DragDropTableView::DragDropTableView(MyQFileSystemModel* fsmModel, QPushButton* mouseSideKeyBackwardBtn, QPushButton* mouseSideKeyForwardBtn)
    : QTableView(),
      View(),
      backwardBtn(mouseSideKeyBackwardBtn),
      forwardBtn(mouseSideKeyForwardBtn),
      m_menu(new RightClickMenu("Right click menu", this)) {
  setModel(fsmModel);
  InitViewSettings();

  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  DragDropTableView::subscribe();

  setStyleSheet(TABLEVIEW_STYLESHEET);
}

void DragDropTableView::subscribe() {
  connect(horizontalHeader(), &QHeaderView::sectionResized, this,
          [this]() { PreferenceSettings().setValue("FILE_EXPLORER_HEADER_GEOMETRY", horizontalHeader()->saveState()); });

  connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &View::onSortIndicatorChanged);
  addActions(g_viewActions()._VIEW_ACRIONS->actions());
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());

  addActions(g_fileBasicOperationsActions().NEW->actions());
  addActions(g_fileBasicOperationsActions().CUT_COPY_MERGE_PASTE->actions());
  addActions(g_fileBasicOperationsActions().FOLDER_MERGE->actions());
  addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
  addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());

  addActions(g_renameAg().RENAME_RIBBONS->actions());

  addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
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
}

void DragDropTableView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, event);
}

void DragDropTableView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, event);
}

void DragDropTableView::dragLeaveEvent(QDragLeaveEvent* event) {
  View::dragLeaveEventCore(this, event);
}

auto DragDropTableView::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier and e->key() == Qt::Key_Delete) {
    emit g_fileBasicOperationsActions().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QTableView::keyPressEvent(e);
}

void DragDropTableView::mousePressEvent(QMouseEvent* event) {
  if (View::onMouseSidekeyBackwardForward(event->button(), backwardBtn, forwardBtn)) {
    return;
  }
  return QTableView::mousePressEvent(event);
}

void DragDropTableView::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::MouseButton::LeftButton) {
    View::mouseMoveEventCore(this, event);
    return;
  }
  return QTableView::mouseMoveEvent(event);
}
