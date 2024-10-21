#include "NavigationViewSwitcher.h"
#include "Component/NotificatorFrame.h"
#include "View/SceneActionsSubscribe.h"
#include "View/SceneTableView.h"
#include "Model/ScenesTableModel.h"
#include "PublicTool.h"
#include "Tools/ActionWithPath.h"
#include "Tools/ViewTypeTool.h"
using namespace ViewTypeTool;

NavigationViewSwitcher::NavigationViewSwitcher(StackedToolBar* navigation, ContentPanel* view, QObject* parent)
    : QObject(parent), _navigation(navigation), _view(view) {}

void NavigationViewSwitcher::onSwitchByViewType(ViewTypeTool::ViewType viewType) {
  // push toolbar to stackwidget and set current toolbar widget
  int naviIndex = -1;
  switch (viewType) {
    case ViewType::LIST:
    case ViewType::TABLE:
    case ViewType::TREE: {
      if (_navigation->m_addressBar == nullptr) {
        _navigation->m_addressBar = new NavigationAndAddressBar;
        _navigation->AddToolBar(ViewType::TABLE, _navigation->m_addressBar);
        _view->BindNavigationAddressBar(_navigation->m_addressBar);

        static auto F_IntoNewPath = [this](QString newPath, bool isNewPath, bool isF5Force) -> bool {
          return _view->onActionAndViewNavigate(newPath, isNewPath, isF5Force);
        };
        static auto F_SearchTextChanged = [this](const QString& targetStr) -> bool { return _view->on_searchTextChanged(targetStr); };
        static auto F_SearchEnterKey = [this](const QString& targetStr) -> bool { return _view->on_searchEnterKey(targetStr); };

        _navigation->m_addressBar->BindFileSystemViewCallback(F_IntoNewPath, F_SearchTextChanged, F_SearchEnterKey, _view->m_fsModel);

        ActionWithPath::BindIntoNewPath(F_IntoNewPath);
      }
      naviIndex = _navigation->m_name2StackIndex[ViewType::TABLE];
      qDebug("Switch toolbar to list/table/tree[%d]", (char)viewType);
      break;
    }
    case ViewType::MOVIE: {
      if (_navigation->m_dbSearchBar == nullptr) {
        _navigation->m_dbSearchBar = new DatabaseSearchToolBar;
        _navigation->AddToolBar(viewType, _navigation->m_dbSearchBar);

        _view->BindDatabaseSearchToolBar(_navigation->m_dbSearchBar);
      }
      naviIndex = _navigation->m_name2StackIndex[viewType];
      qDebug("Switch toolbar to movie[%d]", (char)viewType);
      break;
    }
    case ViewType::SEARCH: {
      if (_navigation->m_advanceSearchBar == nullptr) {
        _navigation->m_advanceSearchBar = new AdvanceSearchToolBar;
        _navigation->AddToolBar(viewType, _navigation->m_advanceSearchBar);

        _view->BindAdvanceSearchToolBar(_navigation->m_advanceSearchBar);
      }
      naviIndex = _navigation->m_name2StackIndex[viewType];
      qDebug("Switch toolbar to search[%d]", (char)viewType);
      break;
    }
    case ViewType::SCENE: {
      if (_navigation->m_addressBar == nullptr) {
        _navigation->m_addressBar = new NavigationAndAddressBar;
        _navigation->AddToolBar(viewType, _navigation->m_addressBar);
      }
      naviIndex = _navigation->m_name2StackIndex[viewType];
      qDebug("Switch toolbar to scene[%d]", (char)viewType);
      break;
    }
    default: {
      return;
    }
  }
  _navigation->m_stackedToolBar->setCurrentIndex(naviIndex);

  // push view widget to stackwidget and set current view widget
  int viewIndex = -1;
  switch (viewType) {
    case ViewType::LIST: {
      if (_view->m_fsListView == nullptr) {
        _view->m_fsListView = new FileSystemListView(_view->m_fsModel);
        ContentPanel::connect(_view->m_fsListView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
        _view->connectSelectionChanged(viewType);
        _view->AddView(viewType, _view->m_fsListView);
      }
      const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
      const QModelIndex& newRootIndex{newPath == _view->m_fsModel->rootPath() ? _view->getRootIndex() : _view->m_fsModel->setRootPath(newPath)};
      if (newRootIndex.isValid()) {
        // sync root index from last valid file-system model root index
        _view->m_fsListView->setRootIndex(newRootIndex);
      }
      viewIndex = _view->m_name2ViewIndex[viewType];
      qDebug("Switch view widget to list[%d]", (char)viewType);
      break;
    }
    case ViewType::TABLE: {
      if (_view->m_fsTableView == nullptr) {
        _view->m_fsTableView = new FileSystemTableView(_view->m_fsModel);
        ContentPanel::connect(_view->m_fsTableView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
        _view->connectSelectionChanged(viewType);
        _view->AddView(viewType, _view->m_fsTableView);
      }
      const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
      const QModelIndex& newRootIndex{newPath == _view->m_fsModel->rootPath() ? _view->getRootIndex() : _view->m_fsModel->setRootPath(newPath)};
      if (newRootIndex.isValid()) {
        // sync root index from last valid file-system model root index
        _view->m_fsTableView->setRootIndex(newRootIndex);
      }
      viewIndex = _view->m_name2ViewIndex[viewType];
      qDebug("Switch view widget to table[%d]", (char)viewType);
      break;
    }
    case ViewType::TREE: {
      if (_view->m_fsTreeView == nullptr) {
        _view->m_fsTreeView = new FileSystemTreeView(_view->m_fsModel);
        ContentPanel::connect(_view->m_fsTreeView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
        _view->connectSelectionChanged(viewType);
        _view->AddView(viewType, _view->m_fsTreeView);
      }
      const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
      const QModelIndex& newRootIndex{newPath == _view->m_fsModel->rootPath() ? _view->getRootIndex() : _view->m_fsModel->setRootPath(newPath)};
      if (newRootIndex.isValid()) {
        // sync root index from last valid file-system model root index
        _view->m_fsTreeView->setRootIndex(newRootIndex);
      }
      viewIndex = _view->m_name2ViewIndex[viewType];
      qDebug("Switch view widget to tree[%d]", (char)viewType);
      break;
    }
    case ViewType::MOVIE: {
      if (_view->m_movieView == nullptr) {
        _view->m_dbModel = new MyQSqlTableModel(_view, GetSqlVidsDB());
        _view->m_movieView = new MovieDBView(_view->_dbSearchBar, _view->m_dbModel, _view);
        ContentPanel::connect(_view->m_movieView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
        _view->AddView(viewType, _view->m_movieView);
      }
      _view->m_movieView->setWindowTitle(QString("Movie[%1]").arg(_view->m_movieView->getMovieTableName()));
      viewIndex = _view->m_name2ViewIndex[viewType];
      qDebug("Switch view widget to movie[%d]", (char)viewType);
      break;
    }
    case ViewType::SEARCH: {
      if (_view->m_advanceSearchView == nullptr) {
        _view->m_searchSrcModel = new AdvanceSearchModel;
        _view->m_proxyModel = new SearchProxyModel;
        _view->m_advanceSearchView = new AdvanceSearchTableView(_view->m_searchSrcModel, _view->m_proxyModel, _view);
        ContentPanel::connect(_view->m_advanceSearchView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);

        _view->m_advanceSearchView->BindLogger(_view->_logger);
        if (_navigation->m_advanceSearchBar != nullptr) {
          _navigation->m_advanceSearchBar->BindSearchAllModel(_view->m_proxyModel, _view->m_searchSrcModel);
        }
        _view->AddView(viewType, _view->m_advanceSearchView);
        _view->m_searchSrcModel->initFilter(
            QDir::Filters{PreferenceSettings().value(MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.name, MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v).toInt()});
      }
      const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
      _view->m_searchSrcModel->setRootPath(newPath);
      _view->m_advanceSearchView->setWindowTitle(QString("Search[%1]").arg(newPath));
      viewIndex = _view->m_name2ViewIndex[viewType];
      qDebug("Switch view widget to search[%d]", (char)viewType);
      break;
    }
    case ViewType::SCENE: {
      if (_view->m_sceneTableView == nullptr) {
        _view->m_scenesModel = new ScenesTableModel;
        _view->m_sceneTableView = new SceneTableView(_view->m_scenesModel, _view);
        ContentPanel::connect(_view->m_sceneTableView, &QAbstractItemView::doubleClicked, _view, &ContentPanel::on_cellDoubleClicked);
        _view->AddView(viewType, _view->m_sceneTableView);
        auto* sceneSub = new (std::nothrow) SceneActionsSubscribe{_view->m_sceneTableView};
        if (sceneSub->BindWidget(_view->m_sceneTableView, _view->m_scenesModel)) {
          sceneSub->operator()();
        }
      }
      const QString& newPath = _navigation->m_addressBar->m_addressLine->pathFromLineEdit();
      _view->m_sceneTableView->setRootPath(newPath);
      viewIndex = _view->m_name2ViewIndex[viewType];
      qDebug("Switch view widget to scene[%d]", (char)viewType);
      break;
    }
    default: {
      return;
    }
  }
  _view->setCurrentIndex(viewIndex);
}

void NavigationViewSwitcher::onSwitchByViewAction(const QAction* activatedAction) {
  ViewType vt = GetViewTypeByActionText(activatedAction);
  onSwitchByViewType(vt);
}
