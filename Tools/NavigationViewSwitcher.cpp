#include "NavigationViewSwitcher.h"
#include "Component/NotificatorFrame.h"
#include "View/SceneTableView.h"
#include "Model/ScenesTableModel.h"
#include "PublicTool.h"
#include "Tools/ActionWithPath.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

NavigationViewSwitcher::NavigationViewSwitcher(StackedToolBar* navigation, ContentPanel* view, QObject* parent)
    : QObject(parent), _navigation(navigation), _view(view) {}

void NavigationViewSwitcher::onSwitchByViewType(const QString& viewType) {
  static const QSet<QString> fileSystemView{"list", "table", "tree"};
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
  } else if (viewType == "scene") {
    if (_navigation->m_addressBar == nullptr) {
      _navigation->m_addressBar = new NavigationAndAddressBar;
      _navigation->AddToolBar("scene", _navigation->m_addressBar);
    }
    naviIndex = _navigation->m_name2StackIndex["scene"];
  }
  _navigation->m_stackedToolBar->setCurrentIndex(naviIndex);

  int viewIndex = -1;
  if (viewType == "table") {
    if (_view->m_fsTableView == nullptr) {
      _view->m_fsTableView = new FileSystemTableView(_view->m_fsModel);
      ContentPanel::connect(_view->m_fsTableView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      _view->connectSelectionChanged(viewType);
      _view->AddView(viewType, _view->m_fsTableView);
    }
    const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
    const auto& newRootIndex = _view->m_fsModel->setRootPath(newPath);
    //    const auto& newRootIndex = _view->getRootIndex();
    if (newRootIndex.isValid()) {
      // sync root index from last valid file-system model root index
      _view->m_fsTableView->setRootIndex(newRootIndex);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "list") {
    if (_view->m_fsListView == nullptr) {
      _view->m_fsListView = new FileSystemListView(_view->m_fsModel);
      ContentPanel::connect(_view->m_fsListView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      _view->connectSelectionChanged(viewType);
      _view->AddView(viewType, _view->m_fsListView);
    }
    const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
    const auto& newRootIndex = _view->m_fsModel->setRootPath(newPath);
//    const auto& newRootIndex = _view->getRootIndex();
    if (newRootIndex.isValid()) {
      // sync root index from last valid file-system model root index
      _view->m_fsListView->setRootIndex(newRootIndex);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "tree") {
    if (_view->m_fsTreeView == nullptr) {
      _view->m_fsTreeView = new FileSystemTreeView(_view->m_fsModel);
      ContentPanel::connect(_view->m_fsTreeView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      _view->connectSelectionChanged(viewType);
      _view->AddView(viewType, _view->m_fsTreeView);
    }
    const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
    const auto& newRootIndex = _view->m_fsModel->setRootPath(newPath);
    //    const auto& newRootIndex = _view->getRootIndex();
    if (newRootIndex.isValid()) {
      // sync root index from last valid file-system model root index
      _view->m_fsTreeView->setRootIndex(newRootIndex);
    }
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "movie") {
    if (_view->m_movieView == nullptr) {
      _view->m_dbModel = new MyQSqlTableModel(_view, GetSqlVidsDB());
      _view->m_movieView = new MovieDBView(_view->_dbSearchBar, _view->m_dbModel, _view);
      ContentPanel::connect(_view->m_movieView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      _view->AddView(viewType, _view->m_movieView);
    }
    _view->m_movieView->setWindowTitle(QString("Movie[%1]").arg(_view->m_movieView->getMovieTableName()));
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "search") {
    if (_view->m_advanceSearchView == nullptr) {
      _view->m_srcModel = new AdvanceSearchModel;
      _view->m_proxyModel = new SearchProxyModel;
      _view->m_advanceSearchView = new AdvanceSearchTableView(_view->m_srcModel, _view->m_proxyModel, _view);
      ContentPanel::connect(_view->m_advanceSearchView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);

      _view->m_advanceSearchView->BindLogger(_view->_logger);
      if (_navigation->m_advanceSearchBar != nullptr) {
        _navigation->m_advanceSearchBar->BindSearchAllModel(_view->m_proxyModel, _view->m_srcModel);
      }
      _view->AddView(viewType, _view->m_advanceSearchView);
      _view->m_srcModel->initFilter(
          QDir::Filters{PreferenceSettings().value(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()});
    }
    const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
    _view->m_srcModel->setRootPath(newPath);
    _view->m_advanceSearchView->setWindowTitle(QString("Search[%1]").arg(newPath));
    viewIndex = _view->m_name2ViewIndex[viewType];
  } else if (viewType == "scene") {
    if (_view->m_sceneTableView == nullptr) {
      _view->m_scenesModel = new ScenesTableModel;
      _view->m_sceneTableView = new SceneTableView(_view->m_scenesModel, _view);
      ContentPanel::connect(_view->m_sceneTableView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
      _view->AddView(viewType, _view->m_sceneTableView);
    }
    const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
    _view->m_sceneTableView->setRootPath(newPath);
    viewIndex = _view->m_name2ViewIndex[viewType];
  }
  _view->setCurrentIndex(viewIndex);
}

void NavigationViewSwitcher::onSwitchByViewAction(const QAction* activatedAction) {
  const QString& viewType = activatedAction->text();
  onSwitchByViewType(viewType);
}
