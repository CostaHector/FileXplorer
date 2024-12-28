#include "ContentPanel.h"
#include "Actions/ArchiveFilesActions.h"
#include "Component/NotificatorFrame.h"
#include "Tools/ArchiveFiles.h"
#include "public/DisplayEnhancement.h"
#include "Model/ScenesTableModel.h"

#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

#include <QAbstractItemModel>
#include <QStandardItem>

#include <QDesktopServices>

#include <QUrl>

using namespace ViewTypeTool;
ContentPanel::ContentPanel(PreviewFolder* previewFolder, QWidget* parent)
  : QStackedWidget(parent), m_fsModel(new MyQFileSystemModel(this)), _previewFolder{previewFolder}, _logger(nullptr), m_parent(parent) {
  layout()->setContentsMargins(0, 0, 0, 0);
  layout()->setSpacing(0);
  subscribe();
}

auto ContentPanel::onActionAndViewNavigate(QString newPath, bool isNewPath, bool /*isF5Force*/) -> bool {
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

bool ContentPanel::onAddressToolbarPathChanged(QString newPath, bool isNewPath) {
  // can only be triggered by lineedit return pressed
  // isNewPath: bool Only differs in undo and redo operation.
  // True means newPath would be push into undo.
  // false not
  if (not newPath.isEmpty() and not QFileInfo(newPath).isDir()) {
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

  if (GetCurViewType() == ViewType::SCENE) {
    if (m_sceneTableView == nullptr) {
      qWarning("m_scenesModel is nullptr");
      return false;
    }
    m_sceneTableView->setRootPath(newPath);
    return true;
  }

  const bool isFileSystem = isFSView();
  if (isFileSystem) {
    QAbstractItemView* fsView = GetCurView();
    if (fsView != nullptr) {
      fsView->setRootIndex(m_fsModel->setRootPath(newPath));
      fsView->selectionModel()->clearCurrentIndex();
      fsView->selectionModel()->clearSelection();
    }
  }
  return true;
}

auto ContentPanel::on_searchTextChanged(const QString& targetStr) -> bool {
  if (GetCurViewType() == ViewType::SCENE) {
    if (m_scenesModel == nullptr) {
      qWarning("m_scenesModel is nullptr");
      return false;
    }
    m_scenesModel->setFilterRegularExpression(targetStr);
    return true;
  }
  if (isFSView()) {
    if (targetStr.isEmpty()) {
      m_fsModel->setNameFilters({});
      return true;
    }
    m_fsModel->setNameFilters({"*" + targetStr + "*"});
  }
  return true;
}

auto ContentPanel::on_searchEnterKey(const QString& targetStr) -> bool {
  if (GetCurViewType() == ViewType::SCENE) {
    if (m_scenesModel == nullptr) {
      qWarning("m_scenesModel is nullptr");
      return false;
    }
    m_scenesModel->setFilterRegularExpression(targetStr);
    return true;
  }
  if (isFSView()) {
    return on_searchTextChanged(targetStr);
  }
  return true;
}

void ContentPanel::subscribe() {
  connect(m_fsModel, &MyQFileSystemModel::directoryLoaded, this, &ContentPanel::onAfterDirectoryLoaded);
}

void ContentPanel::BindNavigationAddressBar(NavigationAndAddressBar* addressBar) {
  if (addressBar == nullptr) {
    qWarning("Bind Navigation AddressBar failed. nullptr passed here");
    return;
  }
  _addressBar = addressBar;
  connect(_addressBar->m_addressLine, &AddressELineEdit::pathActionsTriggeredOrLineEditReturnPressed,
          this, [this](const QString& newPath) { onAddressToolbarPathChanged(newPath, true); });
}

void ContentPanel::BindDatabaseSearchToolBar(DatabaseSearchToolBar* dbSearchBar) {
  if (dbSearchBar == nullptr) {
    qWarning("Bind DatabaseSearchToolBar failed. nullptr passed here");
    return;
  }
  _dbSearchBar = dbSearchBar;
}

void ContentPanel::BindAdvanceSearchToolBar(AdvanceSearchToolBar* advanceSearchBar) {
  if (advanceSearchBar == nullptr) {
    qWarning("Bind AdvanceSearchToolBar failed. nullptr passed here");
    return;
  }
  _advanceSearchBar = advanceSearchBar;
}

void ContentPanel::BindLogger(CustomStatusBar* logger) {
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

auto ContentPanel::on_cellDoubleClicked(const QModelIndex& clickedIndex) -> bool {
  if (not clickedIndex.isValid())
    return false;
  QFileInfo fi = getFileInfo(clickedIndex);
  qInfo("Enter(%d, %d) [%s]", clickedIndex.row(), clickedIndex.column(), qPrintable(fi.fileName()));
  if (not fi.exists()) {
    qWarning("[path inexists] %s", qPrintable(fi.absoluteFilePath()));
    Notificator::warning("Cannot open inexist path", fi.absoluteFilePath());
    return false;
  }
  if (fi.isSymLink()) {
#ifdef _WIN32
    QString tarPath = fi.symLinkTarget();
#else  // ref: https://doc.qt.io/qt-6/qfileinfo.html#isSymLink
    QString tarPath(fi.absoluteFilePath());
#endif
    fi = QFileInfo(tarPath);
    if (not fi.exists()) {
      qDebug("[link inexists] %s", qPrintable(fi.absoluteFilePath()));
      Notificator::warning("Cannot open inexist link", fi.absoluteFilePath());
      return false;
    }
  }

  // For File
  // qz File Open in QZ Archive always
  // other file: open in QDesktopService

  // For Folder
  // FileSystemView: change file system view path to directory.
  // Non-FileSystemView: open in QDesktopService;

  if (fi.isFile()) {
    if (ArchiveFiles::isQZFile(fi)) {
      emit g_AchiveFilesActions().ARCHIVE_PREVIEW->trigger();
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

auto ContentPanel::on_selectionChanged(const QItemSelection& /* selected */, const QItemSelection& /* deselected */) -> bool {
  if (not isFSView()) {
    return false;
  }
  const int selectCnt = getSelectedRowsCount();
  if (_logger) {
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
    if (_logger) _logger->msg(FILE_PROPERTY_DSP::sizeToFileSizeDetail(firstFileInfo.size()));
  } else {
    if (_logger) _logger->msg("size: unknown");
  }

  QString pth = m_fsModel->rootPath();
#ifdef _WIN32
  if (not pth.isEmpty() and pth.back() == ':') {
    pth += '/';
  }
#endif
  m_anchorTags.insert(pth, {firstIndex.row(), firstIndex.column()});
  if (_previewFolder != nullptr) {
    _previewFolder->operator()(firstFileInfo.absoluteFilePath());
  }
  return true;
}

void ContentPanel::connectSelectionChanged(ViewTypeTool::ViewType vt) {
  disconnectSelectionChanged(vt);
  if (vt == ViewType::TABLE) {
    ContentPanel::connect(m_fsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContentPanel::on_selectionChanged);
  } else if (vt == ViewType::LIST) {
    ContentPanel::connect(m_fsListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContentPanel::on_selectionChanged);
  } else if (vt == ViewType::TREE) {
    ContentPanel::connect(m_fsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContentPanel::on_selectionChanged);
  }
}
void ContentPanel::disconnectSelectionChanged(ViewTypeTool::ViewType vt) {
  if (vt == ViewType::TABLE) {
    ContentPanel::disconnect(m_fsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContentPanel::on_selectionChanged);
  } else if (vt == ViewType::LIST) {
    ContentPanel::disconnect(m_fsListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContentPanel::on_selectionChanged);
  } else if (vt == ViewType::TREE) {
    ContentPanel::disconnect(m_fsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContentPanel::on_selectionChanged);
  }
}

bool ContentPanel::onAfterDirectoryLoaded(const QString& loadedPath) {
  qInfo("Directory loaded [%s]", qPrintable(loadedPath));
  if (not m_anchorTags.contains(loadedPath)) {
    qDebug("AnchorTags[%s] not exist. cancel scroll", qPrintable(loadedPath));
    return false;
  }
  const QModelIndex rootIndex = m_fsTableView->rootIndex();
  const QModelIndex anchorInd = m_fsModel->index(m_anchorTags[loadedPath].row, m_anchorTags[loadedPath].col, rootIndex);
  if (not anchorInd.isValid()) {
    qDebug("anchorTags[%s] invalid. cancel scroll", qPrintable(loadedPath));
    m_anchorTags.remove(loadedPath);
    return false;
  }
  m_fsTableView->setCurrentIndex(anchorInd);
  m_fsTableView->scrollTo(anchorInd);
  return true;
}

auto ContentPanel::keyPressEvent(QKeyEvent* e) -> void {
  switch (e->modifiers()) {
    case Qt::KeyboardModifier::NoModifier: {
      switch (e->key()) {
        case Qt::Key_Backspace: {
          if (_addressBar != nullptr) {
            _addressBar->onUpTo();
          }
          return;
        }
        case Qt::Key_Enter:
        case Qt::Key_Return:{ // enter or return
          on_cellDoubleClicked(GetCurView()->currentIndex());
          return;
        }
        default:
          break;
      }
      break;
    }
    default:
      break;
  }
  QStackedWidget::keyPressEvent(e);
}

QModelIndex ContentPanel::getRootIndex() const {
  if (!isFSView()) {
    return QModelIndex();
  }
  return GetCurView()->rootIndex();
}
