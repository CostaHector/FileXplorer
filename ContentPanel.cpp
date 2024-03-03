#include "ContentPanel.h"
#include "Component/RightClickMenu.h"
#include "PublicTool.h"

#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

#include <QAbstractItemModel>
#include <QStandardItem>

#include <QDesktopServices>

#include <QUrl>

auto FileInfoToSearchResult(const QFileInfo& fi, int relativeN) -> QList<QStandardItem*> {
  return {new QStandardItem(fi.filePath().mid(relativeN)), new QStandardItem(fi.fileName()), new QStandardItem(QString::number(fi.size())),
          new QStandardItem(fi.suffix()), new QStandardItem(fi.lastModified().toString("yyyy-MM-dd hh:mm:ss"))};
}

ContentPanel::ContentPanel(FolderPreviewHTML* previewHtml_, FolderPreviewWidget* previewWidget_, QWidget* parent)
    : QStackedWidget(parent),
      _addressBar(nullptr),
      _dbSearchBar(nullptr),
      m_fsModel(new MyQFileSystemModel(this)),
      m_dbModel(new MyQSqlTableModel(this, GetSqlVidsDB())),
      m_menu(new RightClickMenu("Right click menu", this)),
      m_fsView(nullptr),
      m_fsListView(nullptr),
      m_fsTreeView(nullptr),
      m_dbPanel(nullptr),
      previewHtml(previewHtml_),
      previewWidget(previewWidget_),
      _logger(nullptr),
      m_parent(parent) {
  //  onSwitch2ViewType("table");
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
  QAbstractItemView* fsView = GetView();
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
  if (newPath.isEmpty()){
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
  m_fsModel->setNameFilters({});
  //  QString rootPth = CurrentPath();
  //  int n = rootPth.size() + int(not rootPth.isEmpty() and rootPth[rootPth.size() - 1] != "/");  // (pre, rel) rel should not startswith /
  // "C:/" -> n = len = 3
  // "/" -> n = len = 1
  // "C:/home" -> n = len + 1 = 7 + 1
  //  QDirIterator it(rootPth, {"*" + targetStr + "*"}, m_fsm->filter(), QDirIterator::IteratorFlag::Subdirectories);
  //  while (it.hasNext()) {
  //    it.next();
  //    auto lst = FileInfoToSearchResult(it.fileInfo(), n);
  //  }
  qDebug("TODO ADVANCE SEARCH");
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
          [this](const QString& newPath) { onAddressToolbarPathChanged(newPath, true); });
}

void ContentPanel::BindDatabaseSearchToolBar(DatabaseSearchToolBar* dbSearchBar) {
  if (dbSearchBar == nullptr) {
    qWarning("Bind DatabaseSearchToolBar failed. nullptr passed here");
    return;
  }
  _dbSearchBar = dbSearchBar;
}

void ContentPanel::BindCustomStatusBar(CustomStatusBar* logger) {
  if (logger == nullptr) {
    qWarning("Bind CustomStatusBar for contentPanel and FileSystemModel failed. nullptr passed here");
    return;
  }
  _logger = logger;
  m_fsModel->BindLogger(_logger);
}

auto ContentPanel::on_cellDoubleClicked(QModelIndex clickedIndex) -> bool {
  if (not clickedIndex.isValid()) {
    return false;
  }
  QFileInfo fi = m_fsModel->fileInfo(clickedIndex);
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
  if (selected.isEmpty()) {
    return true;
  }
  const QModelIndexList& selectedRowsIndex = selected.indexes();
  // if isinstance(view, QListView):
  // self.view.selectionModel().selectedIndexes()
  // else: self.view.selectionModel().selectedRows() # Table or Tree
  if (_logger) {
    const auto selectedCnt = selectedRowsIndex.size() / 4;  // for table
    _logger->pathInfo(selectedCnt, 1);
  }
  QFileSystemModel* _model = static_cast<QFileSystemModel*>(m_fsView->model());
  const QModelIndex& firstIndex = selectedRowsIndex.front();
  const QFileInfo& firstFileInfo = _model->fileInfo(firstIndex);
  const QString& pth = _model->rootPath();
  m_anchorTags.insert(pth, {firstIndex.row(), firstIndex.column()});
  qDebug("\t\t Anchor of path [%s] target to [%d,%d]", qPrintable(pth), m_anchorTags[pth].row, m_anchorTags[pth].col);
  if (previewWidget) {
    emit previewWidget->showANewPath(firstFileInfo.absoluteFilePath());
  }
  if (previewHtml) {
    previewHtml->operator()(firstFileInfo.absoluteFilePath());
  }
  return true;
}

bool ContentPanel::onAfterDirectoryLoaded(const QString& loadedPath) {
  m_fsView->setFocus();
  qDebug("onAfterDirectoryLoaded[%s]", qPrintable(loadedPath));
  const QModelIndex rootIndex = m_fsView->rootIndex();
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
  m_fsView->setCurrentIndex(qmodelIndex);
  m_fsView->scrollTo(qmodelIndex);
  return true;
}
