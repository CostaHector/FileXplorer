#include "View/FileSystemTableView.h"
#include "View/ViewHelper.h"

#include <QHeaderView>
#include <QMouseEvent>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RenameActions.h"
#include "Actions/ViewActions.h"

FileSystemTableView::FileSystemTableView(MyQFileSystemModel* fsmModel, QMenu* menu, QWidget* parent) : CustomTableView("FILE_SYSTEM", parent) {
  BindMenu(menu);
  setModel(fsmModel);

  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);

  subscribe();

  InitTableView();  
}

void FileSystemTableView::subscribe() {
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

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include <QMainWindow>

class TestVerticalHeaderTable : public QMainWindow {
 public:
  explicit TestVerticalHeaderTable(QWidget* parent = nullptr) : QMainWindow(parent), m_fsm(new QFileSystemModel) {
    auto* tv = new CustomTableView("tmm_");

    tv->setModel(m_fsm);
    tv->setRootIndex(m_fsm->setRootPath("E:/MovieImages/hetero/Babes - Preston & Black Cock Guy"));

    tv->verticalHeader()->setDefaultAlignment(Qt::AlignmentFlag::AlignRight);
    tv->horizontalHeader()->setDefaultAlignment(Qt::AlignmentFlag::AlignRight);

    connect(tv->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this,
            [](int logicalIndex, Qt::SortOrder order) { qDebug() << "Index:" << logicalIndex << "Order:" << order; });

    //    tv->setSortingEnabled(true);

    setCentralWidget(tv);

    setMinimumSize(1024, 768);
  }
  QFileSystemModel* m_fsm;
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  TestVerticalHeaderTable tvh;
  tvh.show();
  return a.exec();
}
#endif
