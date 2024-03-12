#include "AdvanceSearchTableView.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/ViewActions.h"
#include "CustomTableView.h"

AdvanceSearchTableView::AdvanceSearchTableView(AdvanceSearchModel* sourceModel, SearchProxyModel* searchProxyModel, QWidget* parent)
    : CustomTableView("ADVANCE_SEARCH_SYSTEM", parent),
      m_searchMenu{new AdvanceSearchMenu("Search right click menu", this)},
      _sourceModel(sourceModel),
      _searchProxyModel(searchProxyModel) {
  BindMenu(m_searchMenu);

  _searchProxyModel->setSourceModel(_sourceModel);
  setModel(_searchProxyModel);
  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setSortingEnabled(true);

  subscribe();
  InitTableView();
}

void AdvanceSearchTableView::subscribe() {
  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
    if (_logger) {
      _logger->pathInfo(selectionModel()->selectedRows().size(), 1);
    }
  });
  connect(m_searchMenu->_FORCE_REFRESH_SEARCH_SOURCE, &QAction::triggered, _sourceModel, &AdvanceSearchModel::forceRefresh);

  addActions(g_viewActions()._VIDEO_PLAYERS->actions());
  addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
  addActions(g_fileBasicOperationsActions().COPY_PATH_AG->actions());
  addActions(g_fileBasicOperationsActions().CUT_COPY_PASTE->actions());
  addActions(g_fileBasicOperationsActions().DELETE_ACTIONS->actions());
}

void AdvanceSearchTableView::BindLogger(CustomStatusBar* logger) {
  if (logger == nullptr) {
    qWarning("Don't bind nullptr CustomStatusBar to AdvanceSearchTableView/_sourceModel/_searchProxyModel");
    return;
  }
  if (_logger != nullptr) {
    qWarning("Don't rebind to _logger");
    return;
  }
  _logger = logger;
  _sourceModel->BindLogger(_logger);
  //  _searchProxyModel->BindLogger(_logger);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

#include <QMainWindow>

class AdvanceSearchTableViewWindowTest : public QMainWindow {
 public:
  explicit AdvanceSearchTableViewWindowTest(QWidget* parent = nullptr) : QMainWindow(parent) {
    const QString restoredPath = "D:/extra";
    QDir::Filters restoredFilters{
        PreferenceSettings().value(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()};
    m_srcModel->setRootPathAndFilter(restoredPath, restoredFilters);
    m_proxyModel->setSourceModel(m_srcModel);

    m_tv = new AdvanceSearchTableView(m_srcModel, m_proxyModel, this);
    setCentralWidget(m_tv);
    addToolBar(m_tb);
    m_tb->BindSearchAllModel(m_proxyModel, m_srcModel);
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
  AdvanceSearchModel* m_srcModel = new AdvanceSearchModel;
  SearchProxyModel* m_proxyModel = new SearchProxyModel;
  AdvanceSearchTableView* m_tv = nullptr;
};

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  AdvanceSearchTableViewWindowTest mainWindow;
  mainWindow.show();
  a.exec();
  return 0;
}
#endif
