#include "FileSystemListView.h"

#include "View/FileSystemListView.h"

#include "View/ViewHelper.h"

#include <QHeaderView>
#include <QMouseEvent>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RenameActions.h"
#include "Actions/ViewActions.h"
#include "PublicVariable.h"

const QString TABLEVIEW_STYLESHEET = "QListView {"\
    "    show-decoration-selected: 1;"\
    "}"\
    "QListView::item:alternate {"\
    "}"\
    "QListView::item:selected {"\
    "    border-bottom: 1px inherit #FFFFFF;"\
    "}"\
    "QListView::item:selected:!active {"\
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EEEEEE, stop: 1 #999999);"\
    "    color: #000000;"\
    "}"\
    "QListView::item:selected:active {"\
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #99D1FF, stop: 1 #99D1FF);"\
    "    color: #000000;"\
    "    border-top: 2px solid #CCEBFF;"\
    "    border-bottom: 2px solid #CCEBFF;"\
    "}"\
    "QListView::item:hover {"\
    "    background: #CCEBFF;"\
    "}";

FileSystemListView::FileSystemListView(MyQFileSystemModel* fsmModel, QMenu* menu)
    : QListView(),
      _menu(menu){
  setModel(fsmModel);
  InitViewSettings();

  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  FileSystemListView::subscribe();

  setStyleSheet(TABLEVIEW_STYLESHEET);
}

void FileSystemListView::subscribe() {
//  connect(horizontalHeader(), &QHeaderView::sectionResized, this,
//          [this]() { PreferenceSettings().setValue("FILE_EXPLORER_HEADER_GEOMETRY", horizontalHeader()->saveState()); });

//  connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &View::onSortIndicatorChanged);
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

auto FileSystemListView::InitViewSettings() -> void {
//  setShowGrid(false);
  setAlternatingRowColors(true);
//  setSortingEnabled(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);

//  verticalHeader()->setVisible(false);
//  verticalHeader()->setDefaultSectionSize(ROW_SECTION_HEIGHT);
//  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

//  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
//  horizontalHeader()->setStretchLastSection(false);
//  horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);

//  horizontalHeader()->restoreState(PreferenceSettings().value("FILE_EXPLORER_HEADER_GEOMETRY").toByteArray());
  FileSystemListView::UpdateItemViewFontSize();
}

auto FileSystemListView::UpdateItemViewFontSize() -> void {
  View::UpdateItemViewFontSizeCore(this);
}

void FileSystemListView::dropEvent(QDropEvent* event) {
  View::dropEventCore(this, event);
}

void FileSystemListView::dragEnterEvent(QDragEnterEvent* event) {
  View::dragEnterEventCore(this, event);
}

void FileSystemListView::dragMoveEvent(QDragMoveEvent* event) {
  View::dragMoveEventCore(this, event);
}

void FileSystemListView::dragLeaveEvent(QDragLeaveEvent* event) {
  View::dragLeaveEventCore(this, event);
}

auto FileSystemListView::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier and e->key() == Qt::Key_Delete) {
    emit g_fileBasicOperationsActions().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QListView::keyPressEvent(e);
}

void FileSystemListView::mousePressEvent(QMouseEvent* event) {
  if (View::onMouseSidekeyBackwardForward(event->button())) {
    return;
  }
  return QListView::mousePressEvent(event);
}

void FileSystemListView::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::MouseButton::LeftButton) {
    View::mouseMoveEventCore(this, event);
    return;
  }
  return QListView::mouseMoveEvent(event);
}
