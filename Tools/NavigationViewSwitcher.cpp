#include "NavigationViewSwitcher.h"
#include "PublicTool.h"
#include "Tools/ActionWithPath.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

NavigationViewSwitcher::NavigationViewSwitcher(StackedToolBar* navigation, ContentPanel* view, QObject* parent)
    : QObject(parent), _navigation(navigation), _view(view) {}

void NavigationViewSwitcher::onSwitchByViewType(const QString& viewType) {
  const QSet<QString> fileSystemView{"list", "table", "tree"};
  int naviIndex = -1;
  if (fileSystemView.contains(viewType)) {
    if (_navigation->m_addressBar == nullptr) {
      _navigation->m_addressBar = new NavigationAndAddressBar;
      _navigation->AddToolBar("NavigationAddress", _navigation->m_addressBar);
      _view->BindNavigationAddressBar(_navigation->m_addressBar);

      auto F_IntoNewPath = std::bind(&ContentPanel::onActionAndViewNavigate, _view, _1, _2, _3);
      _navigation->m_addressBar->BindFileSystemViewCallback(F_IntoNewPath, std::bind(&ContentPanel::on_searchTextChanged, _view, _1),
                                                            std::bind(&ContentPanel::on_searchEnterKey, _view, _1), _view->m_fsModel);
      ActionWithPath::BindIntoNewPath(F_IntoNewPath);
    }
    naviIndex = _navigation->m_name2StackIndex["NavigationAddress"];
  } else if (viewType == "movie") {
    if (_navigation->m_dbSearchBar == nullptr) {
      _navigation->m_dbSearchBar = new DatabaseSearchToolBar;
      _navigation->AddToolBar("DatabaseSearch", _navigation->m_dbSearchBar);

      _view->BindDatabaseSearchToolBar(_navigation->m_dbSearchBar);
    }
    naviIndex = _navigation->m_name2StackIndex["DatabaseSearch"];
  } else if (viewType == "search") {
    if (_navigation->m_advanceSearchBar == nullptr) {
      _navigation->m_advanceSearchBar = new AdvanceSearchToolBar;
      _navigation->AddToolBar("search", _navigation->m_advanceSearchBar);

      _view->BindAdvanceSearchToolBar(_navigation->m_advanceSearchBar);
    }
    naviIndex = _navigation->m_name2StackIndex["search"];
  }
  _navigation->m_stackedToolBar->setCurrentIndex(naviIndex);

  int viewIndex = -1;
  if (viewType == "table") {
    if (_view->m_fsView == nullptr) {
      _view->m_fsView = new FileSystemTableView(_view->m_fsModel, _view->m_menu);
      ContentPanel::connect(_view->m_fsView, &QTableView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      ContentPanel::connect(_view->m_fsView->selectionModel(), &QItemSelectionModel::selectionChanged, _view, &ContentPanel::on_selectionChanged);
      _view->AddView(viewType, _view->m_fsView);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "list") {
    if (_view->m_fsListView == nullptr) {
      _view->m_fsListView = new FileSystemListView(_view->m_fsModel, _view->m_menu);
      ContentPanel::connect(_view->m_fsListView, &QTableView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      ContentPanel::connect(_view->m_fsListView->selectionModel(), &QItemSelectionModel::selectionChanged, _view, &ContentPanel::on_selectionChanged);
      _view->AddView(viewType, _view->m_fsListView);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "tree") {
    if (_view->m_fsTreeView == nullptr) {
      _view->m_fsTreeView = new FileSystemTreeView(_view->m_fsModel, _view->m_menu);
      ContentPanel::connect(_view->m_fsTreeView, &QTableView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      ContentPanel::connect(_view->m_fsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, _view, &ContentPanel::on_selectionChanged);
      _view->AddView(viewType, _view->m_fsTreeView);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "movie") {
    if (_view->m_dbPanel == nullptr) {
      _view->m_dbModel = new MyQSqlTableModel(_view, GetSqlVidsDB());
      _view->m_dbPanel = new DatabaseTableView(_view->_dbSearchBar, _view->m_dbModel, _view);
      _view->AddView(viewType, _view->m_dbPanel);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "search") {
    if (_view->m_advanceSearchView == nullptr) {
      _view->m_srcModel = new AdvanceSearchModel;
      _view->m_proxyModel = new SearchProxyModel;
      _view->m_advanceSearchView = new AdvanceSearchTableView(_view->m_srcModel, _view->m_proxyModel, _view);
      _view->AddView(viewType, _view->m_advanceSearchView);

      _navigation->m_advanceSearchBar->BindSearchAllModel(_view->m_proxyModel, _view->m_srcModel);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
    const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
    if (newPath.count('/') >= 2) {
      QDir::Filters restoredFilters{
                                    PreferenceSettings().value("FILE_SYSTEM_FLAG_WHEN_FILTER_ENABLED", int(FileSystemTypeFilter::DEFAULT_FILTER_FLAG)).toInt()};
      _view->m_srcModel->setRootPathAndFilter(newPath, restoredFilters);
      _view->m_advanceSearchView->setWindowTitle("Search under|" + newPath);
    } else {
      qWarning("Skip. path[%s] is a huge folder, search will cause lags.", qPrintable(newPath));
    }
  }
  _view->setCurrentIndex(viewIndex);

  if (fileSystemView.contains(viewType) and _view->_addressBar) {
    _view->onActionAndViewNavigate(_view->_addressBar->m_addressLine->pathFromLineEdit(), false);
  }
}

void NavigationViewSwitcher::onSwitchByViewAction(const QAction* activatedAction) {
  const QString& viewType = activatedAction->text();
  onSwitchByViewType(viewType);
}
