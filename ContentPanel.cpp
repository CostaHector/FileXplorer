#include "ContentPanel.h"
#include "Component/RightClickMenu.h"

#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

#include <QAbstractItemModel>
#include <QStandardItem>

#include <QDesktopServices>

#include <QUrl>

ContentPanel::ContentPanel(FolderPreviewHTML* previewHtml_, FolderPreviewWidget* previewWidget_, QWidget* parent)
    : QStackedWidget(parent),
      _addressBar(nullptr),
      _dbSearchBar(nullptr),
      m_fsModel(new MyQFileSystemModel(this)),
      m_menu(new RightClickMenu("Right click menu", this)),
      previewHtml(previewHtml_),
      previewWidget(previewWidget_),
      _logger(nullptr),
      m_parent(parent) {
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
    qDebug("Path[%s] is not [empty or existed directory].", qPrintable(newPath));
    return false;
  }
  QAbstractItemView* fsView = GetCurView();
  if (fsView == nullptr) {
    qWarning("not FileSystemView");
    return false;
  }
  fsView->setRootIndex(m_fsModel->setRootPath(newPath));
  fsView->selectionModel()->clearCurrentIndex();
  fsView->selectionModel()->clearSelection();
  if (m_parent != nullptr) {
    m_parent->setWindowTitle(newPath);
  }
  if (isNewPath) {
    if (_addressBar) {
      _addressBar->m_pathRD(newPath);
    }
  }
#ifdef WIN32
  if (newPath.isEmpty()) {
    onAfterDirectoryLoaded(newPath);
  }
#endif
  return true;
}

auto ContentPanel::on_searchTextChanged(const QString& targetStr) -> bool {
  if (targetStr.isEmpty()) {
    m_fsModel->setNameFilters({});
    return true;
  }
  qDebug("search pattern: [*%s*].", qPrintable(targetStr));
  m_fsModel->setNameFilters({"*" + targetStr + "*"});
  return true;
}

auto ContentPanel::on_searchEnterKey(const QString& targetStr) -> bool {
  return on_searchTextChanged(targetStr);
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
          [this](const QString& newPath) { onAddressToolbarPathChanged(newPath, true); });
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
  qDebug("Enter(%d, %d) [%s]", clickedIndex.row(), clickedIndex.column(), fi.fileName().toStdString().c_str());
  if (not fi.exists()) {
    qDebug("[path inexists] %s", fi.absoluteFilePath().toStdString().c_str());
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
      qDebug("[link inexists] %s", fi.absoluteFilePath().toStdString().c_str());
      return false;
    }
  }
  QString path(fi.absoluteFilePath());
  if (fi.isDir()) {
    onActionAndViewNavigate(path, true, true);
    return true;
  }
  if (fi.isFile()) {
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
  }
  return true;
}

auto ContentPanel::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) -> bool {
  if (m_fsModel == nullptr) {
    return false;
  }
  if (selected.isEmpty()) {
    if (_logger) {
      _logger->pathInfo(0, 1);
    }
    return true;
  }
  // new selected count
  if (_logger) {
    _logger->pathInfo(getSelectedRowsCount(), 1);
  }
  const QModelIndex& firstIndex = selected.indexes().front();
  const QFileInfo& firstFileInfo = m_fsModel->fileInfo(firstIndex);

  const QString& pth = m_fsModel->rootPath();
  m_anchorTags.insert(pth, {firstIndex.row(), firstIndex.column()});
  qDebug("\t\t Anchor of path [%s] target to [%d,%d]", qPrintable(pth), m_anchorTags[pth].row, m_anchorTags[pth].col);
  if (previewWidget != nullptr) {
    emit previewWidget->showANewPath(firstFileInfo.absoluteFilePath());
  }
  if (previewHtml != nullptr) {
    previewHtml->operator()(firstFileInfo.absoluteFilePath());
  }
  return true;
}

bool ContentPanel::onAfterDirectoryLoaded(const QString& loadedPath) {
  m_fsTableView->setFocus();
  qDebug("onAfterDirectoryLoaded[%s]", qPrintable(loadedPath));
  const QModelIndex rootIndex = m_fsTableView->rootIndex();
  if (not m_anchorTags.contains(loadedPath)) {
    qDebug("anchorTags[%s] not exist. scroll abort", qPrintable(loadedPath));
    return false;
  }
  const QModelIndex qmodelIndex = m_fsModel->index(m_anchorTags[loadedPath].row, m_anchorTags[loadedPath].col, rootIndex);
  if (not qmodelIndex.isValid()) {
    qDebug("anchorTags[%s] index invalid. scroll abort", qPrintable(loadedPath));
    m_anchorTags.remove(loadedPath);
    return false;
  }
  m_fsTableView->setCurrentIndex(qmodelIndex);
  m_fsTableView->scrollTo(qmodelIndex);
  return true;
}

auto ContentPanel::keyPressEvent(QKeyEvent* e) -> void {
  if (e->modifiers() == Qt::NoModifier and e->key() == Qt::Key_Backspace) {
    if (_addressBar) {
      _addressBar->onUpTo();
    }
    return;
  } else if (e->modifiers() == Qt::NoModifier and (e->key() == Qt::Key_Enter or e->key() == Qt::Key_Return)) {
    on_cellDoubleClicked(GetCurView()->currentIndex());
    return;
  }
  QStackedWidget::keyPressEvent(e);
}
