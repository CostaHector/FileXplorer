#include "AdvanceSearchTableView.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Component/AdvanceSearchToolBar.h"
#include "Component/FileSystemTypeFilter.h"
#include "CustomTableView.h"
#include "PublicVariable.h"

AdvanceSearchTableView::AdvanceSearchTableView(AdvanceSearchModel* sourceModel, SearchProxyModel* searchProxyModel, QWidget* parent)
    : CustomTableView("ADVANCE_SEARCH_SYSTEM", parent), _sourceModel(sourceModel), _searchProxyModel(searchProxyModel) {
  m_menu->addActions({COPY_NAME_STR, COPY_ABSOLUTE_PATH_STR, COPY_DIR_STR});
  m_menu->addSeparator();
  m_menu->addActions({COPY_FILE, CUT_FILE});
  BindMenu(m_menu);

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

  addActions(g_fileBasicOperationsActions().CUT_COPY_MERGE_PASTE->actions());

  connect(COPY_NAME_STR, &QAction::triggered, this, &AdvanceSearchTableView::onCopyNamesStr);
  connect(COPY_FILE, &QAction::triggered, this, &AdvanceSearchTableView::onCutFiles);
  connect(CUT_FILE, &QAction::triggered, this, &AdvanceSearchTableView::onCopyFiles);
}

void AdvanceSearchTableView::onCopyNamesStr() const {
  qDebug() << "Todo: onCopyNamesStr not support now";
}

void AdvanceSearchTableView::onCopyFiles() {
  QModelIndexList srcCutList;
  for (const auto& ind : selectedIndexes()) {
    const auto indexOfSourceModel = _searchProxyModel->mapToSource(ind);
    srcCutList.append(indexOfSourceModel);
    if (ind.column() != 0) {
      continue;
    }
    qDebug() << "Copy" << ind.row() << _sourceModel->data(indexOfSourceModel, Qt::DisplayRole).toString();
  }
  _sourceModel->CopiedSomething(srcCutList);
}

void AdvanceSearchTableView::onCutFiles() {
  QModelIndexList srcCutList;
  for (const auto& ind : selectedIndexes()) {
    const auto indexOfSourceModel = _searchProxyModel->mapToSource(ind);
    srcCutList.append(indexOfSourceModel);
    if (ind.column() != 0) {
      continue;
    }
    qDebug() << "Cut" << ind.row() << _sourceModel->data(indexOfSourceModel, Qt::DisplayRole).toString();
  }
  _sourceModel->CutSomething(srcCutList);
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

class AdvanceSearchTableViewWindowTest : public QMainWindow {
 public:
  explicit AdvanceSearchTableViewWindowTest(QWidget* parent = nullptr) : QMainWindow(parent) {
    const QString restoredPath = "D:/extra";
    QDir::Filters restoredFilters{
        PreferenceSettings().value("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(FileSystemTypeFilter::DEFAULT_FILTER_FLAG)).toInt()};
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
