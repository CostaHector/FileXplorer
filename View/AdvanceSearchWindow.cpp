#include "AdvanceSearchWindow.h"
#include "PublicVariable.h"

#include "Component/AdvanceSearchToolBar.h"
#include "CustomTableView.h"

AdvanceSearchTableView::AdvanceSearchTableView(MySearchModel* _sourceModel, SearchProxyModel* _searchProxyModel, QWidget* parent)
    : CustomTableView("ADVANCE_SEARCH_SYSTEM", parent) {
  //  BindMenu(menu);
  _searchProxyModel->setSourceModel(_sourceModel);
  setModel(_searchProxyModel);

  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setSortingEnabled(true);

  //  subscribe();
  InitTableView();
}

class AdvanceSearchTableViewWindowTest : public QMainWindow {
 public:
  explicit AdvanceSearchTableViewWindowTest(QWidget* parent = nullptr) : QMainWindow(parent) {
    const QString restoredPath = "D:/extra";
    QDir::Filters restoredFilters{
        PreferenceSettings().value("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(ToolButtonFileSystemTypeFilter::DEFAULT_FILTER_FLAG)).toInt()};
    m_srcModel->setRootPathAndFilter(restoredPath, restoredFilters);
    m_proxyModel->setSourceModel(m_srcModel);

    m_tv = new AdvanceSearchTableView(m_srcModel, m_proxyModel, this);
    setCentralWidget(m_tv);
    addToolBar(m_tb);

    m_tb->BindProxyModel(m_proxyModel);
    m_tb->BindSourceModel(m_srcModel);

    setWindowIcon(QIcon(":/themes/SEARCH"));
    setWindowTitle("Search under|" + m_srcModel->rootPath());
  }
  auto sizeHint() const -> QSize override { return QSize(1024, 768); }

  auto searchInAnotherPath(const QString& newPath) -> bool {
    if (newPath.count('/') <= 2) {
      qWarning("Path is a huge folder, search will cause lags[%s]", qPrintable(newPath));
      return false;
    }
    m_srcModel->setRootPath(newPath);
    setWindowTitle("Search under|" + m_srcModel->rootPath());
    return true;
  }

 private:
  AdvanceSearchToolBar* m_tb = new AdvanceSearchToolBar("advance search tb", this);
  MySearchModel* m_srcModel = new MySearchModel;
  SearchProxyModel* m_proxyModel = new SearchProxyModel;

  AdvanceSearchTableView* m_tv = nullptr;
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  AdvanceSearchTableViewWindowTest mainWindow;
  mainWindow.show();
  a.exec();
  return 0;
}
#endif
