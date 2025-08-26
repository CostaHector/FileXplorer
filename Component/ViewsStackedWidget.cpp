#include "ViewsStackedWidget.h"
#include "ArchiveFilesActions.h"
#include "ViewActions.h"
#include "FolderPreviewActions.h"
#include "Notificator.h"
#include "ArchiveFiles.h"
#include "HarFiles.h"
#include "ScenesListModel.h"
#include "DisplayEnhancement.h"

#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

#include <QAbstractItemModel>
#include <QStandardItem>

#include <QDesktopServices>
#include <QRegularExpression>
#include <QUrl>

using namespace ViewTypeTool;
ViewsStackedWidget::ViewsStackedWidget(SelectionPreviewer* previewFolder, QWidget* parent)
    : QStackedWidget(parent),  //
      mMovieDb{SystemPath::VIDS_DATABASE, "DBMOVIE_CONNECT"},
      mCastDb{SystemPath::PEFORMERS_DATABASE, "CAST_CONNECTION"},
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
  if (not ret) {
    return false;
  }

  if (_addressBar) {
    _addressBar->m_addressLine->updateAddressToolBarPathActions(newPath);
  }
  return true;
}

bool ViewsStackedWidget::onAddressToolbarPathChanged(QString newPath, bool isNewPath) {
  // can only be triggered by lineedit return pressed
  // isNewPath: bool Only differs in undo and redo operation.
  // True means newPath would be push into undo.
  // false not
  if (!newPath.isEmpty() && !QFileInfo(newPath).isDir()) {
    qWarning("Path[%s] is empty or existed directory", qPrintable(newPath));
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
      qWarning("m_scenesModel is nullptr");
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
        qDebug("Not a valid regular expression[%s]", qPrintable(targetStr));
        return false;
      }

      CHECK_NULLPTR_RETURN_FALSE(m_jsonProxyModel);
      m_jsonProxyModel->setFilterRegularExpression(it.value());
      return true;
    }
    default: {
      qWarning("ViewType[%d:%s] not support search text", (int)vt, GetViewTypeHumanFriendlyStr(vt));
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
      qWarning("ViewType[%d:%s] not support search text", (int)vt, GetViewTypeHumanFriendlyStr(vt));
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
    qWarning("Bind CustomStatusBar for contentPanel and FileSystemModel failed. nullptr passed here");
    return;
  }
  if (_logger != nullptr) {
    qWarning("Don't rebind to _logger");
    return;
  }
  _logger = logger;
  m_fsModel->BindLogger(_logger);
}

auto ViewsStackedWidget::on_cellDoubleClicked(const QModelIndex& clickedIndex) -> bool {
  if (!clickedIndex.isValid()) {
    LOG_BAD("Current Index invalid", "double Click skip");
    return false;
  }
  QFileInfo fi = getFileInfo(clickedIndex);
  qInfo("Enter(%d, %d) [%s]", clickedIndex.row(), clickedIndex.column(), qPrintable(fi.fileName()));
  if (!fi.exists()) {
    LOG_BAD("path not exist", fi.absoluteFilePath());
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
      LOG_BAD("link not exists double click not work", tarPath);
      return false;
    }
    qDebug("linked to[%s]", qPrintable(fi.absoluteFilePath()));
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
      emit g_AchiveFilesActions().ARCHIVE_PREVIEW->trigger();
      return true;
    } else if (HarFiles::IsHarFile(fi)) {
      emit g_viewActions()._HAR_VIEW->trigger();
      return true;
    }
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
  }

  if (fi.isDir()) {
    if (isFSView()) {
      return onActionAndViewNavigate(fi.absoluteFilePath(), true, true);
    }
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
  }
  return true;
}

bool ViewsStackedWidget::on_selectionChanged(const QItemSelection& /* selected */, const QItemSelection& /* deselected */) {
  if (!isFSView()) {
    return false;
  }
  const int selectCnt = getSelectedRowsCount();
  if (_logger != nullptr) {
    _logger->pathInfo(selectCnt, 1);
  }
  if (selectCnt <= 0) {
    return false;
  }
  // don't use reference here, indexes() -> QModelIndexList, front() -> const T&
  const QModelIndex firstIndex = GetCurView()->currentIndex();
  if (!firstIndex.isValid()) {
    return false;
  }
  const QFileInfo& firstFileInfo = m_fsModel->fileInfo(firstIndex);
  if (selectCnt == 1 && firstFileInfo.isFile()) {
    if (_logger != nullptr) {
      _logger->msg(FILE_PROPERTY_DSP::sizeToFileSizeDetail(firstFileInfo.size()));
    }
  }

  QString pth = m_fsModel->rootPath();
#ifdef _WIN32
  if (!pth.isEmpty() && pth.back() == ':') {
    pth += '/';
  }
#endif
  m_anchorTags.insert(pth, {firstIndex.row(), firstIndex.column()});
  if (_previewFolder != nullptr && g_folderPreviewActions().PREVIEW_AG->checkedAction() != nullptr) {
    _previewFolder->operator()(firstFileInfo.absoluteFilePath());
  }
  return true;
}

void ViewsStackedWidget::connectSelectionChanged(ViewTypeTool::ViewType vt) {
  disconnectSelectionChanged();
  switch (vt) {
    case ViewType::TABLE:
      mSelectionChangedConn = ViewsStackedWidget::connect(m_fsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ViewsStackedWidget::on_selectionChanged);
      break;
    case ViewType::LIST:
      mSelectionChangedConn = ViewsStackedWidget::connect(m_fsListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ViewsStackedWidget::on_selectionChanged);
      break;
    case ViewType::TREE:
      mSelectionChangedConn = ViewsStackedWidget::connect(m_fsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ViewsStackedWidget::on_selectionChanged);
      break;
    default:
      qDebug("selection changed signal connect skip. current view type[%d]", (int)vt);
  }
}

bool ViewsStackedWidget::onAfterDirectoryLoaded(const QString& loadedPath) {
  if (!m_anchorTags.contains(loadedPath)) {
    return false;
  }
  const QModelIndex rootIndex = m_fsTableView->rootIndex();
  const QModelIndex anchorInd = m_fsModel->index(m_anchorTags[loadedPath].row, m_anchorTags[loadedPath].col, rootIndex);
  if (!anchorInd.isValid()) {
    qDebug("anchorTags[%s] invalid. cancel scroll", qPrintable(loadedPath));
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

QModelIndex ViewsStackedWidget::getRootIndex() const {
  if (!isFSView()) {
    return QModelIndex();
  }
  return GetCurView()->rootIndex();
}
