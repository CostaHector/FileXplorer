#include "ContentPanel.h"
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QStandardItem>

#include <QDesktopServices>

#include <QUrl>

auto FileInfoToSearchResult(const QFileInfo& fi, int relativeN) -> QList<QStandardItem*> {
  return {new QStandardItem(fi.filePath().mid(relativeN)), new QStandardItem(fi.fileName()), new QStandardItem(QString::number(fi.size())),
          new QStandardItem(fi.suffix()), new QStandardItem(fi.lastModified().toString("yyyy-MM-dd hh:mm:ss"))};
}

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

ContentPanel::ContentPanel(QWidget* parent,
                           const QString& defaultPath,
                           FolderPreviewHTML* previewHtml_,
                           FolderPreviewWidget* previewWidget_,
                           CustomStatusBar* _statusBar)
    : QWidget(parent),
      fileSysModel(new MyQFileSystemModel(_statusBar, nullptr)),
      addressBar(new NavigationAndAddressBar(std::bind(&ContentPanel::IntoNewPath, this, _1, _2, _3),
                                             std::bind(&ContentPanel::on_searchTextChanged, this, _1),
                                             std::bind(&ContentPanel::on_searchEnterKey, this, _1))),
      view(new DragDropTableView(fileSysModel, addressBar->backToBtn, addressBar->forwardToBtn)),
      previewHtml(previewHtml_),
      previewWidget(previewWidget_),
      logger(_statusBar) {
  QVBoxLayout* contentPaneLayout = new QVBoxLayout();
  contentPaneLayout->addLayout(addressBar);
  contentPaneLayout->addWidget(view);
  setLayout(contentPaneLayout);

  subscribe();
  IntoNewPath(defaultPath, true);

  contentPaneLayout->setContentsMargins(0, 0, 0, 0);
  contentPaneLayout->setSpacing(0);
}

auto ContentPanel::IntoNewPath(QString newPath, bool isNewPath, bool isF5Force) -> bool {
  // isNewPath: bool Only differs in undo and redo operation.
  // True means newPath would be push into undo.
  // false not
  if (not(newPath.isEmpty() or QFileInfo(newPath).isDir())) {
    return false;
  }
  view->setRootIndex(fileSysModel->setRootPath(newPath));
  addressBar->winAddress->pathChangeTo(newPath);
  view->selectionModel()->clearCurrentIndex();
  view->selectionModel()->clearSelection();
  if (isNewPath) {  // only new path will back to/forwar to
    addressBar->pathRD(newPath);
  }
  fileSysModel->whenRootPathChanged(newPath);
  return true;
}

auto ContentPanel::on_searchTextChanged(const QString& targetStr) -> bool {
  if (targetStr.isEmpty()) {
    fileSysModel->setNameFilters({});
    return true;
  }
  qDebug("search pattern: [*%s*].", targetStr.toStdString().c_str());
  fileSysModel->setNameFilters({"*" + targetStr + "*"});
  return true;
}

auto ContentPanel::on_searchEnterKey(const QString& targetStr) -> bool {
  fileSysModel->setNameFilters({});
  QString rootPth = CurrentPath();
  int n = rootPth.size() + int(not rootPth.isEmpty() and rootPth[rootPth.size() - 1] != "/");  // (pre, rel) rel should not startswith /
  // "C:/" -> n = len = 3
  // "/" -> n = len = 1
  // "C:/home" -> n = len + 1 = 7 + 1
  QDirIterator it(rootPth, {"*" + targetStr + "*"}, fileSysModel->filter(), QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    auto lst = FileInfoToSearchResult(it.fileInfo(), n);
  }
  return true;
}

void ContentPanel::subscribe() {
  connect(view, &QTableView::doubleClicked, this, &ContentPanel::on_cellDoubleClicked);
  connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContentPanel::on_selectionChanged);
  connect(fileSysModel, &MyQFileSystemModel::directoryLoaded, this, &ContentPanel::onAfterDirectoryLoaded);
}

auto ContentPanel::on_cellDoubleClicked(QModelIndex clickedIndex) -> bool {
  if (not clickedIndex.isValid()) {
    return false;
  }
  QFileInfo fi = fileSysModel->fileInfo(clickedIndex);
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
    IntoNewPath(path, true, true);
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
  if (logger) {
    const auto selectedCnt = selectedRowsIndex.size()/4;  // for table
    logger->pathInfo(selectedCnt, 1);
  }
  QFileSystemModel* _model = static_cast<QFileSystemModel*>(view->model());
  const QModelIndex& firstIndex = selectedRowsIndex.front();
  const QFileInfo& firstFileInfo = _model->fileInfo(firstIndex);
  const QString& pth = _model->rootPath();
  m_anchorTags.insert(pth, {firstIndex.row(), firstIndex.column()});
  qDebug("\t\t Anchor of path [%s] target to [%d,%d]", pth.toStdString().c_str(), m_anchorTags[pth].row, m_anchorTags[pth].col);
  emit previewWidget->showANewPath(firstFileInfo.absoluteFilePath());
  return true;
}

bool ContentPanel::onAfterDirectoryLoaded(const QString& loadedPath) {
  view->setFocus();
  const QModelIndex rootIndex = view->rootIndex();
  if (not m_anchorTags.contains(loadedPath)) {
    qDebug("anchorTags[%s] not exist. scroll abort", loadedPath.toStdString().c_str());
    return false;
  }
  const QModelIndex qmodelIndex = fileSysModel->index(m_anchorTags[loadedPath].row, m_anchorTags[loadedPath].col, rootIndex);
  if (not qmodelIndex.isValid()) {
    qDebug("anchorTags[%s] index invalid. scroll abort", loadedPath.toStdString().c_str());
    m_anchorTags.remove(loadedPath);
    return false;
  }
  view->setCurrentIndex(qmodelIndex);
  view->scrollTo(qmodelIndex);
  return true;
}
