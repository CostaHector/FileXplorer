#include "AdvanceSearchMenu.h"

#include "FileBasicOperationsActions.h"
#include "ViewActions.h"

AdvanceSearchMenu::AdvanceSearchMenu(const QString& title, QWidget* parent) : QMenu(title, parent) {
  setToolTipsVisible(true);

  _FORCE_REFRESH_SEARCH_SOURCE->setToolTip("Call QDirIterator to refresh.");

  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
  addSeparator();
  addActions(g_fileBasicOperationsActions().COPY_PATH_AG->actions());
  addSeparator();
  addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
  addSeparator();
  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
  addSeparator();
  addAction(_FORCE_REFRESH_SEARCH_SOURCE);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include <QContextMenuEvent>
#include <QTableView>
#include "AdvanceSearchModel.h"
class AdvanceSearchMenuTest : public QTableView {
 public:
  explicit AdvanceSearchMenuTest(QWidget* parent = nullptr) : QTableView(parent) {
    m_searchModel = new AdvanceSearchModel;
    const QString path = QFileInfo(__FILE__).absolutePath();
    m_searchModel->setRootPathAndFilter(path, QDir::Filters{MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v.toInt()});
    setModel(m_searchModel);

    m_menu = new AdvanceSearchMenu("Search Right click menu", this);
    setWindowTitle("Test Advance Search Menu");
  }
  void contextMenuEvent(QContextMenuEvent* event) override {
    m_menu->popup(mapToGlobal(event->pos()));  // or QCursor::pos()
  }

  auto sizeHint() const -> QSize override { return QSize(600, 400); }

 private:
  AdvanceSearchModel* m_searchModel{nullptr};
  QMenu* m_menu{nullptr};
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  AdvanceSearchMenuTest advMenuTest;
  advMenuTest.show();
  return a.exec();
}
#endif
