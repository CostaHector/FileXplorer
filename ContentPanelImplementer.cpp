#include "ContentPanel.h"

QAbstractItemView* ContentPanel::GetView(const QString& viewType) const {
  if (viewType == "table") {
    return m_fsTableView;
  } else if (viewType == "list") {
    return m_fsListView;
  } else if (viewType == "tree") {
    return m_fsTreeView;
  } else if (viewType == "movie") {
    return m_movieView;
  } else if (viewType == "search") {
    return m_advanceSearchView;
  }
  qDebug("ViewType[%s] not found", qPrintable(viewType));
  return nullptr;
}

QString ContentPanel::GetCurViewName() const {
  const int curViewInd = currentIndex();
  for (const auto& name : m_name2ViewIndex.keys()) {
    if (curViewInd == m_name2ViewIndex[name]) {
      return name;
    }
  }
  qDebug("View name at index[%d] not found", curViewInd);
  return "";
}

int ContentPanel::AddView(const QString& viewType, QWidget* w) {
  return m_name2ViewIndex[viewType] = addWidget(w);
}

QString ContentPanel::getRootPath() const {
  const QString& viewName = GetCurViewName();
  if (viewName == "search" or viewName == "table" or viewName == "list" or viewName == "tree") {
    return m_fsModel->rootPath();
  }
  qDebug("No rootpath");
  return "";
}

QString ContentPanel::getFilePath(const QModelIndex& ind) const {
  const QString& viewName = GetCurViewName();
  if (viewName == "table" or viewName == "list" or viewName == "tree") {
    return m_fsModel->filePath(ind);
  }
  if (viewName == "movie") {
    return m_dbModel->filePath(ind);
  }
  if (viewName == "search") {
    const auto srcIndex = m_proxyModel->mapToSource(ind);
    return m_srcModel->filePath(srcIndex);
  }
  qDebug("No FilePath");
  return "";
}

QModelIndexList ContentPanel::getSelectedRows() const {
  const QString& viewName = GetCurViewName();
  if (viewName == "table") {
    return m_fsTableView->selectionModel()->selectedRows();
  }
  if (viewName == "list") {
    return m_fsListView->selectionModel()->selectedRows();
  }
  if (viewName == "tree") {
    return m_fsTreeView->selectionModel()->selectedRows();
  }
  if (viewName == "movie") {
    return m_movieView->selectionModel()->selectedRows();
  }
  if (viewName == "search") {
    const auto& proxyIndexesLst = m_advanceSearchView->selectionModel()->selectedRows();
    QModelIndexList srcIndexesLst;
    for (const auto& ind : proxyIndexesLst) {
      srcIndexesLst.append(m_proxyModel->mapToSource(ind));
    }
    return srcIndexesLst;
  }
  qDebug("No SelectedRows");
  return {};
}

QStringList ContentPanel::getFileNames() const {
  const QString& viewName = GetCurViewName();
  QStringList names;
  if (viewName == "table") {
    for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
      names.append(m_fsModel->fileName(ind));
    }
  } else if (viewName == "list") {
    for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
      names.append(m_fsModel->fileName(ind));
    }
  } else if (viewName == "tree") {
    for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
      names.append(m_fsModel->fileName(ind));
    }
  } else if (viewName == "movie") {
    for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
      names.append(m_dbModel->fileName(ind));
    }
  } else if (viewName == "search") {
    for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
      const auto& srcIndex = m_proxyModel->mapToSource(ind);
      names.append(m_srcModel->fileName(srcIndex));
    }
  } else {
    qDebug("No getFileNames");
  }
  return names;
}

QStringList ContentPanel::getFullRecords() const {
  const QString& viewName = GetCurViewName();
  QStringList fullRecords;
  if (viewName == "table") {
    for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
      fullRecords.append(m_fsModel->fullInfo(ind));
    }
  } else if (viewName == "list") {
    for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
      fullRecords.append(m_fsModel->fullInfo(ind));
    }
  } else if (viewName == "tree") {
    for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
      fullRecords.append(m_fsModel->fullInfo(ind));
    }
  } else if (viewName == "movie") {
    for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
      fullRecords.append(m_dbModel->fullInfo(ind));
    }
  } else if (viewName == "search") {
    for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
      const auto& srcIndex = m_proxyModel->mapToSource(ind);
      fullRecords.append(m_srcModel->fullInfo(srcIndex));
    }
  } else {
    qDebug("No getFullRecords");
  }
  return fullRecords;
}

QStringList ContentPanel::getFilePaths() const {
  const QString& viewName = GetCurViewName();
  QStringList filePaths;
  if (viewName == "table") {
    for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
      filePaths.append(m_fsModel->filePath(ind));
    }
  } else if (viewName == "list") {
    for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
      filePaths.append(m_fsModel->filePath(ind));
    }
  } else if (viewName == "tree") {
    for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
      filePaths.append(m_fsModel->filePath(ind));
    }
  } else if (viewName == "movie") {
    for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
      filePaths.append(m_dbModel->filePath(ind));
    }
  } else if (viewName == "search") {
    for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
      const auto& srcIndex = m_proxyModel->mapToSource(ind);
      filePaths.append(m_srcModel->filePath(srcIndex));
    }
  } else {
    qDebug("No getFilePaths");
  }
  return filePaths;
}

QStringList ContentPanel::getFilePrepaths() const {
  const QString& viewName = GetCurViewName();
  QStringList prepaths;
  if (viewName == "table") {
    int rowCnt = m_fsTableView->selectionModel()->selectedRows().size();
    const QString& prepath = m_fsModel->rootPath();
    prepaths.reserve(rowCnt);
    while (rowCnt-- > 0) {
      prepaths.append(prepath);
    }
  } else if (viewName == "list") {
    int rowCnt = m_fsListView->selectionModel()->selectedRows().size();
    const QString& prepath = m_fsModel->rootPath();
    prepaths.reserve(rowCnt);
    while (rowCnt-- > 0) {
      prepaths.append(prepath);
    }
  } else if (viewName == "tree") {
    int rowCnt = m_fsTreeView->selectionModel()->selectedRows().size();
    const QString& prepath = m_fsModel->rootPath();
    prepaths.reserve(rowCnt);
    while (rowCnt-- > 0) {
      prepaths.append(prepath);
    }
  } else if (viewName == "movie") {
    for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
      prepaths.append(m_dbModel->absolutePath(ind));
    }
  } else if (viewName == "search") {
    for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
      const auto& srcIndex = m_proxyModel->mapToSource(ind);
      prepaths.append(m_srcModel->absolutePath(srcIndex));
    }
  } else {
    qDebug("No getFilePrepaths");
  }
  return prepaths;
}

QStringList ContentPanel::getTheJpgFolderPaths() const {
  const QString& viewName = GetCurViewName();

  QStringList prepaths;
  if (viewName == "table") {
    for (const auto& ind : m_fsTableView->selectionModel()->selectedRows()) {
      const QFileInfo dirFi = m_fsModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      prepaths.append(QDir::toNativeSeparators(imagePath));
    }
  } else if (viewName == "list") {
    for (const auto& ind : m_fsListView->selectionModel()->selectedRows()) {
      const QFileInfo dirFi = m_fsModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      prepaths.append(QDir::toNativeSeparators(imagePath));
    }
  } else if (viewName == "tree") {
    for (const auto& ind : m_fsTreeView->selectionModel()->selectedRows()) {
      const QFileInfo dirFi = m_fsModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      prepaths.append(QDir::toNativeSeparators(imagePath));
    }
  } else if (viewName == "movie") {
    for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
      const QFileInfo dirFi = m_dbModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      prepaths.append(m_dbModel->absolutePath(ind));
    }
  } else if (viewName == "search") {
    for (const auto& ind : m_advanceSearchView->selectionModel()->selectedRows()) {
      const auto& srcIndex = m_proxyModel->mapToSource(ind);
      const QFileInfo dirFi = m_srcModel->fileInfo(ind);
      const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
      prepaths.append(m_srcModel->absolutePath(srcIndex));
    }
  } else {
    qDebug("No getTheJpgFolderPaths");
  }
  return prepaths;
}

std::pair<QStringList, QList<QUrl>> ContentPanel::getFilePathsAndUrls(const Qt::DropAction dropAct) const {
  const QString& viewName = GetCurViewName();
  QStringList filePaths;
  QList<QUrl> urls;
  if (viewName == "table") {
    const auto& inds = m_fsTableView->selectionModel()->selectedRows();
    for (const auto& ind : inds) {
      filePaths.append(m_fsModel->filePath(ind));
      urls.append(QUrl::fromLocalFile(filePaths.back()));
    }
    if (dropAct == Qt::CopyAction) {
      m_fsModel->CopiedSomething(inds);
    } else if (dropAct == Qt::MoveAction) {
      m_fsModel->CutSomething(inds);
    }
  } else if (viewName == "list") {
    const auto& inds = m_fsListView->selectionModel()->selectedRows();
    for (const auto& ind : inds) {
      filePaths.append(m_fsModel->filePath(ind));
      urls.append(QUrl::fromLocalFile(filePaths.back()));
    }
    if (dropAct == Qt::CopyAction) {
      m_fsModel->CopiedSomething(inds);
    } else if (dropAct == Qt::MoveAction) {
      m_fsModel->CutSomething(inds);
    }
  } else if (viewName == "tree") {
    const auto& inds = m_fsTreeView->selectionModel()->selectedRows();
    for (const auto& ind : inds) {
      filePaths.append(m_fsModel->filePath(ind));
      urls.append(QUrl::fromLocalFile(filePaths.back()));
    }
    if (dropAct == Qt::CopyAction) {
      m_fsModel->CopiedSomething(inds);
    } else if (dropAct == Qt::MoveAction) {
      m_fsModel->CutSomething(inds);
    }
  } else if (viewName == "movie") {
    for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
      filePaths.append(m_dbModel->filePath(ind));
      urls.append(QUrl::fromLocalFile(filePaths.back()));
    }
  } else if (viewName == "search") {
    QModelIndexList srcInds;
    for (const auto& proInd : m_advanceSearchView->selectionModel()->selectedRows()) {
      const auto& ind = m_proxyModel->mapToSource(proInd);
      srcInds.append(ind);
      filePaths.append(m_srcModel->filePath(ind));
      urls.append(QUrl::fromLocalFile(filePaths.back()));
    }
    if (dropAct == Qt::CopyAction) {
      m_srcModel->CopiedSomething(srcInds);
    } else if (dropAct == Qt::MoveAction) {
      m_srcModel->CutSomething(srcInds);
    }
  } else {
    qDebug("No getFilePathsAndUrls");
  }
  return {filePaths, urls};
}

std::pair<QStringList, QStringList> ContentPanel::getFilePrepathsAndName(const bool isSearchRecycle, const bool needSyncModelDisplay) const {
  const QString& viewName = GetCurViewName();
  QStringList prepaths;
  QStringList names;
  prepaths.reserve(10);
  names.reserve(10);
  if (viewName == "table") {
    const QString prepath = m_fsModel->rootPath();
    const auto& inds = m_fsTableView->selectionModel()->selectedRows();
    for (const auto& ind : inds) {
      prepaths.append(prepath);
      names.append(m_fsModel->fileName(ind));
    }
  } else if (viewName == "list") {
    const QString prepath = m_fsModel->rootPath();
    const auto& inds = m_fsListView->selectionModel()->selectedRows();
    for (const auto& ind : inds) {
      prepaths.append(prepath);
      names.append(m_fsModel->fileName(ind));
    }
  } else if (viewName == "tree") {
    const QString prepath = m_fsModel->rootPath();
    const auto& inds = m_fsTreeView->selectionModel()->selectedRows();
    for (const auto& ind : inds) {
      prepaths.append(prepath);
      names.append(m_fsModel->fileName(ind));
    }
  } else if (viewName == "movie") {
    for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
      prepaths.append(m_dbModel->absolutePath(ind));
      names.append(m_dbModel->fileName(ind));
    }
  } else if (viewName == "search") {
    QSet<QModelIndex> srcInds;
    for (const auto& proInd : m_advanceSearchView->selectionModel()->selectedRows()) {
      const auto& ind = m_proxyModel->mapToSource(proInd);
      prepaths.append(m_srcModel->absolutePath(ind));
      names.append(m_srcModel->fileName(ind));
      srcInds.insert(ind);
    }
    if (isSearchRecycle) {
      m_srcModel->RecycleSomething(srcInds);
    }
  } else {
    qDebug("No getFilePrepathsAndName");
  }
  if (prepaths.size() != names.size()) {
    qWarning("getFilePrepathsAndName size differ");
    return {};
  }
  return {prepaths, names};
}

int ContentPanel::getSelectedRowsCount() const {
  const QString& viewName = GetCurViewName();
  if (viewName == "table") {
    return m_fsTableView->selectionModel()->selectedRows().size();
  } else if (viewName == "list") {
    return m_fsListView->selectionModel()->selectedRows().size();
  } else if (viewName == "tree") {
    return m_fsTreeView->selectionModel()->selectedRows().size();
  } else if (viewName == "movie") {
    return m_movieView->selectionModel()->selectedRows().size();
  } else if (viewName == "search") {
    return m_advanceSearchView->selectionModel()->selectedRows().size();
  }
  qDebug("No getSelectedRowsCount");
  return -1;
}

QString ContentPanel::getCurFilePath() const {
  const QString& viewName = GetCurViewName();
  if (viewName == "table") {
    return m_fsModel->filePath(m_fsTableView->currentIndex());
  } else if (viewName == "list") {
    return m_fsModel->filePath(m_fsListView->currentIndex());
  } else if (viewName == "tree") {
    return m_fsModel->filePath(m_fsTreeView->currentIndex());
  } else if (viewName == "movie") {
    return m_dbModel->filePath(m_movieView->currentIndex());
  } else if (viewName == "search") {
    return m_srcModel->filePath(m_proxyModel->mapToSource(m_advanceSearchView->currentIndex()));
  }
  qDebug("No getCurFilePath");
  return "";
}

QFileInfo ContentPanel::getFileInfo(const QModelIndex& ind) const {
  const QString& viewName = GetCurViewName();
  if (viewName == "table") {
    return m_fsModel->fileInfo(ind);
  } else if (viewName == "list") {
    return m_fsModel->fileInfo(ind);
  } else if (viewName == "tree") {
    return m_fsModel->fileInfo(ind);
  } else if (viewName == "movie") {
    return m_dbModel->fileInfo(ind);
  } else if (viewName == "search") {
    return m_srcModel->fileInfo(m_proxyModel->mapToSource(ind));
  }
  qDebug("No getFileInfo");
  return QFileInfo();
}
