#include "ViewsStackedWidget.h"
#include "ViewActions.h"
#include "ArchiveFiles.h"
#include "ArchiveFilesActions.h"
#include "FolderPreviewActions.h"
#include "NotificatorMacro.h"
#include "HarFiles.h"
#include "ViewTypeTool.h"
#include "DataFormatter.h"
#include "ScenesListModel.h"

#include <QDesktopServices>
#include <QRegularExpression>
#include <QUrl>

using namespace ViewTypeTool;
ViewsStackedWidget::ViewsStackedWidget(CurrentRowPreviewer* previewFolder, QWidget* parent)
    : QStackedWidget(parent),  //
      mMovieDb{SystemPath::VIDS_DATABASE(), "DBMOVIE_CONNECT"},
      mCastDb{SystemPath::PEFORMERS_DATABASE(), "CAST_CONNECTION"},
      _previewFolder{previewFolder},  //
      m_parent(parent)                //
{
  m_fsModel = new (std::nothrow) FileSystemModel(this);
  layout()->setContentsMargins(0, 0, 0, 0);
  layout()->setSpacing(0);
  subscribe();
  setFocusPolicy(Qt::StrongFocus);
}

auto ViewsStackedWidget::onActionAndViewNavigate(QString newPath, bool isNewPath, bool /*isF5Force*/) -> bool {
  // can only be triggered by path action clicked/upTo/backTo/backTo/table view double clicked
  bool ret = onAddressToolbarPathChanged(newPath, isNewPath);
  if (!ret) {
    return false;
  }
  if (_addressBar != nullptr) {
    _addressBar->m_addressLine->updateAddressToolBarPathActions(newPath);
  }
  return true;
}

bool ViewsStackedWidget::onAddressToolbarPathChanged(QString newPath, bool isNewPath) {
  // can only be triggered by lineedit return pressed
  // isNewPath: bool Only differs in undo and redo operation.
  // True means newPath would be push into undo otherwise not
  if (!newPath.isEmpty() && !QFileInfo(newPath).isDir()) { // may be an shared folder cross platform
    LOG_W("Path[%s] is empty or existed directory", qPrintable(newPath));
    return false;
  }

  if (m_parent != nullptr) {
    m_parent->setWindowTitle(newPath);
  }

  if (isNewPath) {
    if (_addressBar != nullptr) {
      _addressBar->m_pathRD(newPath);
    }
  }

  if (GetVt() == ViewType::SCENE) {
    if (m_sceneTableView == nullptr) {
      LOG_W("m_scenesModel is nullptr");
      return false;
    }
    m_sceneTableView->setRootPath(newPath);
    return true;
  }

  ViewTypeTool::ViewType vt = GetVt();
  switch (vt) {
    case ViewType::LIST:
    case ViewType::TABLE:
    case ViewType::TREE: {
      QAbstractItemView* fsView = GetCurView();
      if (fsView != nullptr) {
        fsView->setRootIndex(m_fsModel->setRootPath(newPath));
        fsView->selectionModel()->clearCurrentIndex();
        fsView->selectionModel()->clearSelection();
      }
      break;
    }
    case ViewType::JSON: {
      if (m_jsonTableView != nullptr) {
        m_jsonTableView->ReadADirectory(newPath);
      }
      break;
    }
    default:
      break;
  }
  return true;
}

auto ViewsStackedWidget::on_searchTextChanged(const QString& targetStr) -> bool {
  const ViewTypeTool::ViewType vt{GetVt()};

  switch (vt) {
    case ViewType::LIST:
    case ViewType::TABLE:
    case ViewType::TREE: {
      CHECK_NULLPTR_RETURN_FALSE(m_fsModel);
      m_fsModel->setNameFilters({'*' + targetStr + '*'});
      return true;
    }
    case ViewType::SCENE: {
      CHECK_NULLPTR_RETURN_FALSE(m_scenesModel);
      m_scenesModel->setFilterRegularExpression(targetStr);
      return true;
    }
    case ViewType::JSON: {
      static QHash<QString, QRegularExpression> exprHash;
      auto it = exprHash.constFind(targetStr);
      if (it == exprHash.cend()) {
        QRegularExpression expr{targetStr};
        it = exprHash.insert(targetStr, expr);
      }
      if (!it->isValid()) {
        LOG_D("Not a valid regular expression[%s]", qPrintable(targetStr));
        return false;
      }

      CHECK_NULLPTR_RETURN_FALSE(m_jsonProxyModel);
      m_jsonProxyModel->setFilterRegularExpression(it.value());
      return true;
    }
    default: {
      LOG_W("ViewType[%d:%s] not support search text", (int)vt, c_str(vt));
      return false;
    }
  }
  return true;
}

auto ViewsStackedWidget::on_searchEnterKey(const QString& /*targetStr*/) -> bool {
  const ViewTypeTool::ViewType vt{GetVt()};
  switch (vt) {
    case ViewType::LIST:
    case ViewType::TABLE:
    case ViewType::TREE:
    case ViewType::JSON: {
      // ignore
      return true;
    }
    case ViewType::SCENE: {
      // ignore
      return true;
    }
    default: {
      LOG_W("ViewType[%d:%s] not support search text", (int)vt, c_str(vt));
      return false;
    }
  }
  return true;
}

void ViewsStackedWidget::subscribe() {
  connect(m_fsModel, &FileSystemModel::directoryLoaded, this, &ViewsStackedWidget::onAfterDirectoryLoaded);
}

void ViewsStackedWidget::BindNavigationAddressBar(NavigationAndAddressBar* addressBar) {
  CHECK_NULLPTR_RETURN_VOID(addressBar)
  _addressBar = addressBar;
  connect(_addressBar->m_addressLine, &AddressELineEdit::pathActionsTriggeredOrLineEditReturnPressed,  //
          this,                                                                                        //
          [this](const QString& newPath) {                                                             //
            onAddressToolbarPathChanged(newPath, true);                                                //
          }                                                                                            //
  );
}

void ViewsStackedWidget::BindDatabaseSearchToolBar(MovieDBSearchToolBar* dbSearchBar) {
  CHECK_NULLPTR_RETURN_VOID(dbSearchBar)
  _movieSearchBar = dbSearchBar;
}

void ViewsStackedWidget::BindAdvanceSearchToolBar(AdvanceSearchToolBar* advanceSearchBar) {
  CHECK_NULLPTR_RETURN_VOID(advanceSearchBar)
  _advanceSearchBar = advanceSearchBar;
}

void ViewsStackedWidget::BindCastSearchToolBar(CastDatabaseSearchToolBar* castSearchBar) {
  CHECK_NULLPTR_RETURN_VOID(castSearchBar)
  _castSearchBar = castSearchBar;
}

void ViewsStackedWidget::BindLogger(CustomStatusBar* logger) {
  if (logger == nullptr) {
    LOG_W("Bind CustomStatusBar for contentPanel and FileSystemModel failed. nullptr passed here");
    return;
  }
  if (_logger != nullptr) {
    LOG_W("Don't rebind to _logger");
    return;
  }
  _logger = logger;
  m_fsModel->BindLogger(_logger);
}

auto ViewsStackedWidget::on_cellDoubleClicked(const QModelIndex& clickedIndex) -> bool {
  if (!clickedIndex.isValid()) {
    LOG_ERR_NP("Current Index invalid", "double Click skip");
    return false;
  }
  QFileInfo fi = getFileInfo(clickedIndex);
  LOG_I("Enter(%d, %d) [%s]", clickedIndex.row(), clickedIndex.column(), qPrintable(fi.fileName()));
  if (!fi.exists()) {
    LOG_ERR_NP("path not exist", fi.absoluteFilePath());
    return false;
  }
  if (fi.isSymLink()) {
#ifdef _WIN32
    const QString tarPath{fi.symLinkTarget()};
#else  // ref: https://doc.qt.io/qt-6/qfileinfo.html#isSymLink
    const QString tarPath{fi.absoluteFilePath()};
#endif
    fi.setFile(tarPath);
    if (!fi.exists()) {
      LOG_ERR_NP("link not exists double click not work", tarPath);
      return false;
    }
    LOG_D("linked to[%s]", qPrintable(fi.absoluteFilePath()));
  }

  // For File
  // qz File Open in QZ Archive always
  // har File Open in Har Viewer always
  // other file: open in QDesktopService

  // For Folder
  // FileSystemView: change file system view path to directory.
  // Non-FileSystemView: open in QDesktopService;

  if (fi.isFile()) {
    if (ArchiveFiles::isQZFile(fi)) {
      emit g_AchiveFilesActions().ARCHIVE_PREVIEW->triggered();
      return true;
    } else if (HarFiles::IsHarFile(fi)) {
      emit g_viewActions()._HAR_VIEW->triggered();
      return true;
    }
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
  }

  if (fi.isDir()) {
    ViewTypeTool::ViewType vt = GetVt();
    if (ViewTypeTool::isFSView(vt)) {
      return onActionAndViewNavigate(fi.absoluteFilePath(), true, true);
    }
    if (ViewTypeTool::IsMatch(vt, (int)ViewTypeTool::ViewTypeMask::CAST)) {
      g_viewActions()._TABLE_VIEW->setChecked(true);
      emit g_viewActions()._TABLE_VIEW->triggered(true);  // both 2 signals: 1) undo stack+1 and 2) view Switched happen
      return onActionAndViewNavigate(fi.absoluteFilePath(), true, true);
    }
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
  }
  return true;
}

void ViewsStackedWidget::on_fsmCurrentRowChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
  // don't use reference here, indexes() -> QModelIndexList, front() -> const T&
  if (!current.isValid()) {
    return;
  }
  const QFileInfo fi = getFileInfo(current);
  if (_logger != nullptr && fi.isFile()) {
    _logger->onMsgChanged(DataFormatter::formatFileSizeWithBytes(fi.size()));
  }

  auto vt = GetVt();
  if (ViewTypeTool::isFSView(vt)) {  // anchorTags only work for file-system
    QString parentPth = fi.absolutePath();
#ifdef _WIN32
    if (!parentPth.isEmpty() && parentPth.back() == ':') {
      parentPth += '/';
    }
#endif
    m_anchorTags.insert(parentPth, {current.row(), current.column()});
  }

  if (_previewFolder != nullptr && _previewFolder->GetCurrentViewE() != PreviewTypeTool::PREVIEW_TYPE_E::NONE) {
    _previewFolder->operator()(fi.absoluteFilePath());
  }
}

void ViewsStackedWidget::on_selectionChanged(const QItemSelection& selected, const QItemSelection& /* deselected */) {
  if (_logger != nullptr) {
    _logger->onPathInfoChanged(getSelectedRowsCount(), 1);
  }
}

void ViewsStackedWidget::connectSelectionChanged(ViewTypeTool::ViewType vt) {
  disconnectSelectionChanged();

  auto* curView = GetCurView();
  mSelectionChangedConn =
      ViewsStackedWidget::connect(curView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ViewsStackedWidget::on_selectionChanged);
  mDoubleClickedConnectConn =
      ViewsStackedWidget::connect(curView, &QAbstractItemView::doubleClicked, this, &ViewsStackedWidget::on_cellDoubleClicked);
  switch (vt) {
    case ViewType::LIST:
    case ViewType::TABLE:
    case ViewType::TREE:
    case ViewType::SEARCH: {
      mCurrentChangedConn = ViewsStackedWidget::connect(curView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                                                        &ViewsStackedWidget::on_fsmCurrentRowChanged);
      break;
    }
    case ViewType::SCENE: {
      mCurrentChangedConn = ViewsStackedWidget::connect(
          m_sceneTableView, &SceneListView::currentSceneChanged,
          _previewFolder,  //
          static_cast<void (CurrentRowPreviewer::*)(const QString&, const QStringList&, const QStringList&)>(&CurrentRowPreviewer::operator()));
      break;
    }
    case ViewType::CAST: {
      mCurrentChangedConn =
          ViewsStackedWidget::connect(m_castTableView, &CastDBView::currentRecordChanged,
                                      _previewFolder,  //
                                      static_cast<void (CurrentRowPreviewer::*)(const QSqlRecord&, const QString)>(&CurrentRowPreviewer::operator()));
      break;
    }
    default: {
      LOG_D("[Skip] viewType[%s] current row change signal", ViewTypeTool::c_str(vt));
      return;
    }
  }
}

bool ViewsStackedWidget::onAfterDirectoryLoaded(const QString& loadedPath) {
  if (!m_anchorTags.contains(loadedPath)) {
    return false;
  }
  const QModelIndex rootIndex = m_fsTableView->rootIndex();
  const QModelIndex anchorInd = m_fsModel->index(m_anchorTags[loadedPath].row, m_anchorTags[loadedPath].col, rootIndex);
  if (!anchorInd.isValid()) {
    LOG_D("anchorTags[%s] invalid. cancel scroll", qPrintable(loadedPath));
    m_anchorTags.remove(loadedPath);
    return false;
  }
  m_fsTableView->setCurrentIndex(anchorInd);
  m_fsTableView->scrollTo(anchorInd);
  return true;
}

auto ViewsStackedWidget::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier) {
    switch (e->key()) {
      case Qt::Key_Backspace: {
        if (_addressBar != nullptr) {
          _addressBar->onUpTo();
        }
        return;
      }
      case Qt::Key_Return: {
        on_cellDoubleClicked(GetCurView()->currentIndex());
        return;
      }
      default:
        break;
    }
  }
  QStackedWidget::keyPressEvent(e);
}

std::pair<QModelIndex, QModelIndex> ViewsStackedWidget::getTopLeftAndRightDownRectangleIndex() const {
  static const auto GetterForFsModel = [](const QAbstractItemModel* model,
                                          const QModelIndex& rootIndex) -> decltype(getTopLeftAndRightDownRectangleIndex()) {
    return {model->index(0, 0, rootIndex), model->index(model->rowCount(rootIndex) - 1, model->columnCount(rootIndex) - 1, rootIndex)};
  };
  // attention here we get the proxy rectangle index, not the source model index. for display it is ok.
  // but for records contents, mapToSource Needed Here
  auto vt = GetVt();
  switch (vt) {
    case ViewType::LIST:
      return GetterForFsModel(m_fsModel, m_fsListView->rootIndex());
    case ViewType::TABLE:
      return GetterForFsModel(m_fsModel, m_fsTableView->rootIndex());
    case ViewType::TREE:
      return GetterForFsModel(m_fsModel, m_fsTreeView->rootIndex());
    case ViewType::SEARCH:
      return GetterForFsModel(m_searchProxyModel, m_advanceSearchView->rootIndex());
    case ViewType::MOVIE:
      return GetterForFsModel(m_movieDbModel, m_movieView->rootIndex());
    case ViewType::SCENE:
      return GetterForFsModel(m_scenesModel, m_sceneTableView->rootIndex());
    case ViewType::CAST:
      return GetterForFsModel(m_castDbModel, m_castTableView->rootIndex());
    case ViewType::JSON:
      return GetterForFsModel(m_jsonProxyModel, m_jsonTableView->rootIndex());
    default:
      LOG_W("No left-top to right-down rectangle index in ViewType[%d]", int(vt));
      return {};
  }
}

QModelIndex ViewsStackedWidget::getRootIndex() const {
  if (!IsCurFSView()) {
    return QModelIndex();
  }
  return GetCurView()->rootIndex();
}

const char* ViewsStackedWidget::GetCurViewName() const {
  return ViewTypeTool::c_str(GetVt());
}

int ViewsStackedWidget::AddView(ViewType vt, QWidget* w) {
  return m_name2ViewIndex[vt] = addWidget(w);
}

bool ViewsStackedWidget::hasSelection() const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::LIST:
      return m_fsListView->selectionModel()->hasSelection();
    case ViewType::TABLE:
      return m_fsTableView->selectionModel()->hasSelection();
    case ViewType::TREE:
      return m_fsTreeView->selectionModel()->hasSelection();
    case ViewType::SEARCH:
      return m_advanceSearchView->selectionModel()->hasSelection();
    case ViewType::MOVIE:
      return m_movieView->selectionModel()->hasSelection();
    case ViewType::SCENE:
      return m_sceneTableView->selectionModel()->hasSelection();
    case ViewType::CAST:
      return m_castTableView->selectionModel()->hasSelection();
    case ViewType::JSON:
      return m_jsonTableView->selectionModel()->hasSelection();
    default:
      LOG_W("No SelectedRows in ViewType[%d]", int(vt));
      return false;
  }
}

QString ViewsStackedWidget::getRootPath() const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::LIST:
    case ViewType::TABLE:
    case ViewType::TREE:
    case ViewType::SEARCH:
    case ViewType::SCENE:
    case ViewType::JSON: {
      return m_fsModel->rootPath();
    }
    case ViewType::CAST:
      return m_castDbModel->rootPath();
    default:
      LOG_W("No rootpath in ViewType[%d]", int(vt));
      return "";
  }
}

QString ViewsStackedWidget::getFilePath(const QModelIndex& ind) const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE:
    case ViewType::LIST:
    case ViewType::TREE:
      return m_fsModel->filePath(ind);
    case ViewType::SEARCH: {
      const auto srcIndex = m_searchProxyModel->mapToSource(ind);
      return m_searchSrcModel->filePath(srcIndex);
    }
    case ViewType::MOVIE:
      return m_movieDbModel->filePath(ind);
    case ViewType::SCENE:
      return m_scenesModel->filePath(ind);
    case ViewType::CAST:
      return m_castDbModel->filePath(ind);
    case ViewType::JSON: {
      const auto& srcIndex = m_jsonProxyModel->mapToSource(ind);
      return m_jsonModel->filePath(srcIndex);
    }
    default:
      LOG_W("No FilePath in ViewType[%d]", int(vt));
      return "";
  }
}

QModelIndexList ViewsStackedWidget::getSelectedRows() const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE:
      return m_fsTableView->selectionModel()->selectedRows();
    case ViewType::LIST:
      return m_fsListView->selectionModel()->selectedRows();
    case ViewType::TREE:
      return m_fsTreeView->selectionModel()->selectedRows();
    case ViewType::SEARCH: {
      const auto& proxyIndexesLst = m_advanceSearchView->selectionModel()->selectedRows();
      QModelIndexList srcIndexesLst;
      for (const auto& ind : proxyIndexesLst) {
        srcIndexesLst.append(m_searchProxyModel->mapToSource(ind));
      }
      return srcIndexesLst;
    }
    case ViewType::SCENE: {
      return {};
    }
    case ViewType::MOVIE:
      return m_movieView->selectionModel()->selectedRows();
    case ViewType::JSON:
      return m_jsonTableView->selectionModel()->selectedRows();
    default:
      LOG_W("No SelectedRows in ViewType[%d]", int(vt));
      return {};
  }
}

QStringList ViewsStackedWidget::getFileNames() const {
  QStringList names;
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE: {
      for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
        names.append(m_fsModel->fileName(ind));
      }
      break;
    }
    case ViewType::LIST: {
      for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
        names.append(m_fsModel->fileName(ind));
      }
      break;
    }
    case ViewType::TREE: {
      for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
        names.append(m_fsModel->fileName(ind));
      }
      break;
    }
    case ViewType::SEARCH: {
      for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_searchProxyModel->mapToSource(ind);
        names.append(m_searchSrcModel->fileName(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedRows()) {
        names.append(m_scenesModel->fileName(ind));
      }
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        names.append(m_movieDbModel->fileName(ind));
      }
      break;
    }
    case ViewType::JSON: {
      for (const auto& proInd : m_jsonTableView->selectionModel()->selectedRows()) {
        const auto& ind = m_jsonProxyModel->mapToSource(proInd);
        names.append(m_jsonModel->fileName(ind));
      }
      break;
    }
    default:
      LOG_W("No SelectedRows in ViewType[%d]", int(vt));
      return {};
  }
  return names;
}

QStringList ViewsStackedWidget::getFullRecords() const {
  QStringList fullRecords;
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE: {
      for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
        fullRecords.append(m_fsModel->fullInfo(ind));
      }
      break;
    }
    case ViewType::LIST: {
      for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
        fullRecords.append(m_fsModel->fullInfo(ind));
      }
      break;
    }
    case ViewType::TREE: {
      for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
        fullRecords.append(m_fsModel->fullInfo(ind));
      }
      break;
    }
    case ViewType::SEARCH: {
      for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_searchProxyModel->mapToSource(ind);
        fullRecords.append(m_searchSrcModel->fullInfo(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      LOG_D("Todo: getFullRecords is not supported in scene model now");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        fullRecords.append(m_movieDbModel->fullInfo(ind));
      }
      break;
    }
    case ViewType::JSON: {
      for (const auto& ind : m_jsonTableView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_jsonProxyModel->mapToSource(ind);
        fullRecords.append(m_jsonModel->fullInfo(srcIndex));
      }
      break;
    }
    default: {
      LOG_W("No getFullRecords ViewType:%d", (int)vt);
      break;
    }
  }
  return fullRecords;
}

QStringList ViewsStackedWidget::getFilePaths() const {
  QStringList filePaths;
  auto vt = GetVt();
  switch (vt) {
    case ViewType::LIST: {
      for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
        filePaths.append(m_fsModel->filePath(ind));
      }
      break;
    }
    case ViewType::TABLE: {
      for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
        filePaths.append(m_fsModel->filePath(ind));
      }
      break;
    }
    case ViewType::TREE: {
      for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
        filePaths.append(m_fsModel->filePath(ind));
      }
      break;
    }
    case ViewType::SEARCH: {
      for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_searchProxyModel->mapToSource(ind);
        filePaths.append(m_searchSrcModel->filePath(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedRows()) {
        filePaths.append(m_scenesModel->filePath(ind));
      }
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        filePaths.append(m_movieDbModel->filePath(ind));
      }
      break;
    }
    case ViewType::JSON: {
      for (const auto& ind : m_jsonTableView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_jsonProxyModel->mapToSource(ind);
        filePaths.append(m_jsonModel->filePath(srcIndex));
      }
      break;
    }
    default: {
      LOG_D("No getFilePaths ViewType:%d", (int)vt);
      break;
    }
  }
  return filePaths;
}

QStringList ViewsStackedWidget::getFilePrepaths() const {
  QStringList prepaths;
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE: {
      int rowCnt = m_fsTableView->selectionModel()->selectedRows().size();
      const QString& prepath = m_fsModel->rootPath();
      prepaths.reserve(rowCnt);
      while (rowCnt-- > 0) {
        prepaths.append(prepath);
      }
      break;
    }
    case ViewType::LIST: {
      int rowCnt = m_fsListView->selectionModel()->selectedRows().size();
      const QString& prepath = m_fsModel->rootPath();
      prepaths.reserve(rowCnt);
      while (rowCnt-- > 0) {
        prepaths.append(prepath);
      }
      break;
    }
    case ViewType::TREE: {
      int rowCnt = m_fsTreeView->selectionModel()->selectedRows().size();
      const QString& prepath = m_fsModel->rootPath();
      prepaths.reserve(rowCnt);
      while (rowCnt-- > 0) {
        prepaths.append(prepath);
      }
      break;
    }
    case ViewType::SEARCH: {
      for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_searchProxyModel->mapToSource(ind);
        prepaths.append(m_searchSrcModel->absolutePath(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedRows()) {
        prepaths.append(m_scenesModel->absolutePath(ind));
      }
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        prepaths.append(m_movieDbModel->absolutePath(ind));
      }
      break;
    }
    case ViewType::JSON: {
      for (const auto& ind : m_jsonTableView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_jsonProxyModel->mapToSource(ind);
        prepaths.append(m_jsonModel->absolutePath(srcIndex));
      }
      break;
    }
    default: {
      LOG_D("No getFilePrepaths ViewType:%d", (int)vt);
      break;
    }
  }
  return prepaths;
}

QStringList ViewsStackedWidget::getTheJpgFolderPaths() const {
  QStringList prepaths;
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE: {
      for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
        const QFileInfo dirFi = m_fsModel->fileInfo(ind);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(QDir::toNativeSeparators(imagePath));
      }
      break;
    }
    case ViewType::LIST: {
      for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
        const QFileInfo dirFi = m_fsModel->fileInfo(ind);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(QDir::toNativeSeparators(imagePath));
      }
      break;
    }
    case ViewType::TREE: {
      for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
        const QFileInfo dirFi = m_fsModel->fileInfo(ind);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(QDir::toNativeSeparators(imagePath));
      }
      break;
    }
    case ViewType::SEARCH: {
      for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_searchProxyModel->mapToSource(ind);
        const QFileInfo dirFi = m_searchSrcModel->fileInfo(srcIndex);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(imagePath);
      }
      break;
    }
    case ViewType::SCENE: {
      LOG_D("Todo: need complement");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        const QFileInfo dirFi = m_movieDbModel->fileInfo(ind);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(imagePath);
      }
      break;
    }
    case ViewType::JSON: {
      for (const auto& ind : m_jsonTableView->selectionModel()->selectedRows()) {
        const auto& srcIndex = m_jsonProxyModel->mapToSource(ind);
        const QFileInfo dirFi = m_jsonModel->fileInfo(srcIndex);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(imagePath);
      }
      break;
    }
    default: {
      LOG_D("No getTheJpgFolderPaths");
    }
  }
  return prepaths;
}

MimeDataHelper::MimeDataMember ViewsStackedWidget::getFilePathsAndUrls(const Qt::DropAction dropAct) const {
  using namespace MimeDataHelper;
  auto vt = GetVt();
  switch (vt) {
    case ViewType::LIST: {
      MimeDataMember ret = GetMimeDataMemberFromSourceModel<FileSystemModel>(*m_fsModel, m_fsListView->selectionModel()->selectedRows());
      FillCutCopySomething<FileSystemModel>(*m_fsModel, ret.srcIndexes, dropAct);
      return ret;
    }
    case ViewType::TABLE: {
      MimeDataMember ret = GetMimeDataMemberFromSourceModel<FileSystemModel>(*m_fsModel, m_fsTableView->selectionModel()->selectedRows());
      FillCutCopySomething<FileSystemModel>(*m_fsModel, ret.srcIndexes, dropAct);
      return ret;
    }
    case ViewType::TREE: {
      MimeDataMember ret = GetMimeDataMemberFromSourceModel<FileSystemModel>(*m_fsModel, m_fsTreeView->selectionModel()->selectedRows());
      FillCutCopySomething<FileSystemModel>(*m_fsModel, ret.srcIndexes, dropAct);
      return ret;
    }
    case ViewType::SEARCH: {
      MimeDataMember ret = GetMimeDataMemberFromSearchModel(*m_searchSrcModel, *m_searchProxyModel, m_advanceSearchView->selectionModel()->selectedRows());
      FillCutCopySomething<AdvanceSearchModel>(*m_searchSrcModel, ret.srcIndexes, dropAct);
      return ret;
    }
    case ViewType::SCENE: {
      LOG_D("SCENE type not support getFilePathsAndUrls");
      return {};
    }
    case ViewType::MOVIE: {
      return GetMimeDataMemberFromSourceModel<FdBasedDbModel>(*m_movieDbModel, m_movieView->selectionModel()->selectedRows());
    }
    default: {
      LOG_W("ViewType:%s not support getFilePathsAndUrls", c_str(vt));
      return {};
    }
  }
}

std::pair<QStringList, QStringList> ViewsStackedWidget::getFilePrepathsAndName(const bool isSearchRecycle) const {
  QStringList prepaths;
  QStringList names;
  prepaths.reserve(10);
  names.reserve(10);
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE: {
      const QString prepath = m_fsModel->rootPath();
      const auto& inds = m_fsTableView->selectionModel()->selectedRows();
      for (const auto& ind : inds) {
        prepaths.append(prepath);
        names.append(m_fsModel->fileName(ind));
      }
      break;
    }
    case ViewType::LIST: {
      const QString prepath = m_fsModel->rootPath();
      const auto& inds = m_fsListView->selectionModel()->selectedRows();
      for (const auto& ind : inds) {
        prepaths.append(prepath);
        names.append(m_fsModel->fileName(ind));
      }
      break;
    }
    case ViewType::TREE: {
      const QString prepath = m_fsModel->rootPath();
      const auto& inds = m_fsTreeView->selectionModel()->selectedRows();
      for (const auto& ind : inds) {
        prepaths.append(prepath);
        names.append(m_fsModel->fileName(ind));
      }
      break;
    }
    case ViewType::SEARCH: {
      QSet<QModelIndex> srcInds;
      for (const auto& proInd : m_advanceSearchView->selectionModel()->selectedRows()) {
        const auto& ind = m_searchProxyModel->mapToSource(proInd);
        prepaths.append(m_searchSrcModel->absolutePath(ind));
        names.append(m_searchSrcModel->fileName(ind));
        srcInds.insert(ind);
      }
      if (isSearchRecycle) {
        m_searchSrcModel->RecycleSomething(srcInds);
      }
      break;
    }
    case ViewType::SCENE: {
      LOG_D("Todo getFilePrepathsAndName");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        prepaths.append(m_movieDbModel->absolutePath(ind));
        names.append(m_movieDbModel->fileName(ind));
      }
      break;
    }
    case ViewType::JSON: {
      QSet<QModelIndex> srcInds;
      for (const auto& proInd : m_jsonTableView->selectionModel()->selectedRows()) {
        const auto& ind = m_jsonProxyModel->mapToSource(proInd);
        prepaths.append(m_jsonModel->absolutePath(ind));
        names.append(m_jsonModel->fileName(ind));
        srcInds.insert(ind);
      }
      break;
    }
    default: {
      LOG_D("No getFilePrepathsAndName");
      break;
    }
  }

  if (prepaths.size() != names.size()) {
    LOG_W("getFilePrepathsAndName size differ");
    return {};
  }
  return {prepaths, names};
}

int ViewsStackedWidget::getSelectedRowsCount() const {
  auto* curView = GetCurView();
  if (curView == nullptr) {
    LOG_D("curView[%s] is nullptr", ViewTypeTool::c_str(GetVt()));
    return -1;
  }
  return curView->selectionModel()->selectedRows().size();
}

QString ViewsStackedWidget::getCurFilePath() const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE: {
      return m_fsModel->filePath(m_fsTableView->currentIndex());
    }
    case ViewType::LIST: {
      return m_fsModel->filePath(m_fsListView->currentIndex());
    }
    case ViewType::TREE: {
      return m_fsModel->filePath(m_fsTreeView->currentIndex());
    }
    case ViewType::SEARCH: {
      return m_searchSrcModel->filePath(m_searchProxyModel->mapToSource(m_advanceSearchView->currentIndex()));
    }
    case ViewType::SCENE: {
      return m_scenesModel->filePath(m_sceneTableView->currentIndex());
    }
    case ViewType::CAST: {
      return m_castDbModel->filePath(m_castTableView->currentIndex());
    }
    case ViewType::MOVIE: {
      return m_movieDbModel->filePath(m_movieView->currentIndex());
    }
    case ViewType::JSON: {
      return m_jsonModel->filePath(m_jsonProxyModel->mapToSource(m_jsonTableView->currentIndex()));
    }
    default: {
      LOG_W("No getCurFilePath");
    }
  }
  LOG_W("nothing selected in viewType[%d]", (int)vt);
  return "";
}

QString ViewsStackedWidget::getCurFileName() const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE: {
      return m_fsModel->fileName(m_fsTableView->currentIndex());
    }
    case ViewType::LIST: {
      return m_fsModel->fileName(m_fsListView->currentIndex());
    }
    case ViewType::TREE: {
      return m_fsModel->fileName(m_fsTreeView->currentIndex());
    }
    case ViewType::SEARCH: {
      return m_searchSrcModel->fileName(m_searchProxyModel->mapToSource(m_advanceSearchView->currentIndex()));
    }
    case ViewType::MOVIE: {
      return m_movieDbModel->fileName(m_movieView->currentIndex());
    }
    case ViewType::SCENE: {
      return m_scenesModel->fileName(m_sceneTableView->currentIndex());
    }
    case ViewType::JSON: {
      return m_jsonModel->fileName(m_jsonProxyModel->mapToSource(m_jsonTableView->currentIndex()));
    }
    default: {
      LOG_W("No getCurFileName");
    }
  }
  return "";
}

QFileInfo ViewsStackedWidget::getFileInfo(const QModelIndex& ind) const {
  const auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE:
    case ViewType::LIST:
    case ViewType::TREE: {
      return m_fsModel->fileInfo(ind);
    }
    case ViewType::SEARCH: {
      return m_searchSrcModel->fileInfo(m_searchProxyModel->mapToSource(ind));
    }
    case ViewType::MOVIE: {
      return m_movieDbModel->fileInfo(ind);
    }
    case ViewType::SCENE: {
      return m_scenesModel->fileInfo(ind);
    }
    case ViewType::CAST: {
      return m_castDbModel->fileInfo(ind);
    }
    case ViewType::JSON: {
      return m_jsonModel->fileInfo(m_jsonProxyModel->mapToSource(ind));
    }
    default: {
      LOG_D("ViewType[%s] does not support fileInfo", ViewTypeTool::c_str(vt));
      break;
    }
  }
  return {};
}
