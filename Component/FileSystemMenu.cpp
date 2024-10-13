#include "FileSystemMenu.h"
#include "Actions/ArchiveFilesActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RenameActions.h"
#include "Actions/ViewActions.h"

FileSystemMenu::FileSystemMenu(const QString& title, QWidget* parent) : QMenu(title, parent), NEW_MENU{GetNewMenu()}, VIEW_MENU{GetViewMenu()} {
  setToolTipsVisible(true);

  addActions(g_viewActions()._VIDEO_PLAYERS->actions());
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
  addAction(g_AchiveFilesActions().ARCHIVE_PREVIEW);
  addSeparator();
  addActions(g_fileBasicOperationsActions().COPY_PATH_AG->actions());
  addSeparator();

  addMenu(NEW_MENU);
  addMenu(VIEW_MENU);
  addSeparator();
  addActions(g_fileBasicOperationsActions().MOVE_COPY_TO->actions());
  addSeparator();

  addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
  addSeparator();
  addActions(g_fileBasicOperationsActions().FOLDER_MERGE->actions());
  addSeparator();

  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
  addMenu(GetRenameMenu());
}

QMenu* FileSystemMenu::GetNewMenu() {
  auto* _newMenuLevel2 = new QMenu(tr("&New"), this);
  _newMenuLevel2->setIcon(QIcon(":img/NEW_FILE_FOLDER_PATH"));
  _newMenuLevel2->setToolTipsVisible(true);
  _newMenuLevel2->addActions(g_fileBasicOperationsActions().NEW->actions());
  return _newMenuLevel2;
}

QMenu* FileSystemMenu::GetViewMenu() {
  auto* viewMenuL2 = new QMenu(tr("&View"), this);
  viewMenuL2->setIcon(QIcon(":img/SORTING_FILE_FOLDER"));
  viewMenuL2->setToolTipsVisible(true);
  //    viewMenuL2->addActions(g_viewActions().SORT_INDICATOR_ORDER->actions());
  //  viewMenuL2->addSeparator();
  //  viewMenuL2->addActions(g_viewActions().SORT_INDICATOR_LOGICAL_INDEX->actions());
  //  viewMenuL2->addSeparator();
  //  viewMenuL2->addActions(g_viewActions().TRIPLE_VIEW->actions());
  //  viewMenuL2->addSeparator();
  //  viewMenuL2->addActions(g_viewActions().LISTVIEW_VIEWMODE->actions());
  //  viewMenuL2->addSeparator();
  //  viewMenuL2->addActions(g_viewActions().LISTVIEW_FLOW->actions());
  //  viewMenuL2->addSeparator();
  //  viewMenuL2->addActions(g_viewActions().LISTVIEW_GRIDSIZE_LEVEL->actions());
  //  viewMenuL2->addSeparator();
  return viewMenuL2;
}

QMenu* FileSystemMenu::GetRenameMenu() {
  auto* renameMenuLevel2 = new QMenu(tr("&Rename"), this);
  renameMenuLevel2->setIcon(QIcon(":img/RENAME"));
  renameMenuLevel2->addActions(g_renameAg().RENAME_RIBBONS->actions());
  return renameMenuLevel2;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QListView>
class FileSystemMenuTest : public QListView {
 public:
  explicit FileSystemMenuTest(QWidget* parent = nullptr) : QListView(parent) {
    m_fileSysModel = new QFileSystemModel;
    m_fileSysModel->setReadOnly(true);
    setModel(m_fileSysModel);

    m_menu = new FileSystemMenu("File System menu", this);
    setWindowTitle("Test File System Menu");

    const QString path = QFileInfo(__FILE__).absolutePath();
    m_fileSysModel->setRootPath(path);
    setRootIndex(m_fileSysModel->index(path));
  }
  void contextMenuEvent(QContextMenuEvent* event) override {
    m_menu->popup(mapToGlobal(event->pos()));  // or QCursor::pos()
  }

  auto sizeHint() const -> QSize override { return QSize(600, 400); }

 private:
  QFileSystemModel* m_fileSysModel{nullptr};
  QMenu* m_menu{nullptr};
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FileSystemMenuTest filesytemMenuTest;
  filesytemMenuTest.show();
  return a.exec();
}
#endif
