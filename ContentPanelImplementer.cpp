#include "ContentPanel.h"
#include "Model/ScenesTableModel.h"

using namespace ViewTypeTool;
/*
  auto vt = GetCurViewType();
  switch (vt) {
    case ViewType::TABLE: {
    }
    case ViewType::LIST: {
    }
    case ViewType::TREE: {
    }
    case ViewType::SEARCH: {
    }
    case ViewType::SCENE: {
    }
    case ViewType::MOVIE:{
    }
    default: {
    }
  }

 */

QString ContentPanel::GetCurViewName() const {
  ViewType vt = GetCurViewType();
  const QString& viewName = GetViewTypeHumanFriendlyStr(vt);
  if (viewName.isEmpty()) {
    qCritical("View name[%c] not found", char(vt));
  }
  return viewName;
}

int ContentPanel::AddView(ViewType vt, QWidget* w) {
  return m_name2ViewIndex[vt] = addWidget(w);
}

QString ContentPanel::getRootPath() const {
  auto vt = GetCurViewType();
  switch (vt) {
    case ViewType::TABLE:
    case ViewType::LIST:
    case ViewType::TREE:
    case ViewType::SEARCH:
    case ViewType::SCENE:
      return m_fsModel->rootPath();
    default:
      qWarning("No rootpath in ViewType[%c]", char(vt));
      return "";
  }
}

QString ContentPanel::getFilePath(const QModelIndex& ind) const {
  auto vt = GetCurViewType();
  switch (vt) {
    case ViewType::TABLE:
    case ViewType::LIST:
    case ViewType::TREE:
      return m_fsModel->filePath(ind);
    case ViewType::SEARCH: {
      const auto srcIndex = m_proxyModel->mapToSource(ind);
      return m_searchSrcModel->filePath(srcIndex);
    }
    case ViewType::SCENE:
      return m_scenesModel->filePath(ind);
    case ViewType::MOVIE:
      return m_dbModel->filePath(ind);
    default:
      qWarning("No FilePath in ViewType[%c]", char(vt));
      return "";
  }
}

QModelIndexList ContentPanel::getSelectedRows() const {
  auto vt = GetCurViewType();
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
        srcIndexesLst.append(m_proxyModel->mapToSource(ind));
      }
      return srcIndexesLst;
    }
    case ViewType::SCENE: {
      return {};
    }
    case ViewType::MOVIE:
      return m_movieView->selectionModel()->selectedRows();
    default:
      qWarning("No SelectedRows in ViewType[%c]", char(vt));
      return {};
  }
}

QStringList ContentPanel::getFileNames() const {
  QStringList names;
  auto vt = GetCurViewType();
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
        const auto& srcIndex = m_proxyModel->mapToSource(ind);
        names.append(m_searchSrcModel->fileName(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedIndexes()) {
        names.append(m_scenesModel->fileName(ind));
      }
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        names.append(m_dbModel->fileName(ind));
      }
      break;
    }
    default:
      qWarning("No SelectedRows in ViewType[%c]", char(vt));
      return {};
  }
  return names;
}

QStringList ContentPanel::getFullRecords() const {
  QStringList fullRecords;
  auto vt = GetCurViewType();
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
        const auto& srcIndex = m_proxyModel->mapToSource(ind);
        fullRecords.append(m_searchSrcModel->fullInfo(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      qDebug("Todo: getFullRecords is not supported in scene model now");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        fullRecords.append(m_dbModel->fullInfo(ind));
      }
      break;
    }
    default: {
      qDebug("No getFullRecords");
      break;
    }
  }
  return fullRecords;
}

QStringList ContentPanel::getFilePaths() const {
  QStringList filePaths;
  auto vt = GetCurViewType();
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
        const auto& srcIndex = m_proxyModel->mapToSource(ind);
        filePaths.append(m_searchSrcModel->filePath(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedIndexes()) {
        filePaths.append(m_scenesModel->filePath(ind));
      }
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        filePaths.append(m_dbModel->filePath(ind));
      }
      break;
    }
    default: {
      qDebug("No getFilePaths");
      break;
    }
  }
  return filePaths;
}

QStringList ContentPanel::getFilePrepaths() const {
  QStringList prepaths;
  auto vt = GetCurViewType();
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
        const auto& srcIndex = m_proxyModel->mapToSource(ind);
        prepaths.append(m_searchSrcModel->absolutePath(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedIndexes()) {
        prepaths.append(m_scenesModel->absolutePath(ind));
      }
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        prepaths.append(m_dbModel->absolutePath(ind));
      }
      break;
    }
    default: {
      qDebug("No getFilePrepaths");
      break;
    }
  }
  return prepaths;
}

QStringList ContentPanel::getTheJpgFolderPaths() const {
  QStringList prepaths;
  auto vt = GetCurViewType();
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
        const auto& srcIndex = m_proxyModel->mapToSource(ind);
        const QFileInfo dirFi = m_searchSrcModel->fileInfo(ind);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(m_searchSrcModel->absolutePath(srcIndex));
      }
      break;
    }
    case ViewType::SCENE: {
      qDebug("Todo: need complement");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        const QFileInfo dirFi = m_dbModel->fileInfo(ind);
        const QString& imagePath = QDir(dirFi.absoluteFilePath()).absoluteFilePath(dirFi.fileName() + ".jpg");
        prepaths.append(m_dbModel->absolutePath(ind));
      }
      break;
    }
    default: {
      qDebug("No getTheJpgFolderPaths");
    }
  }
  return prepaths;
}

std::pair<QStringList, QList<QUrl>> ContentPanel::getFilePathsAndUrls(const Qt::DropAction dropAct) const {
  QStringList filePaths;
  QList<QUrl> urls;
  auto vt = GetCurViewType();
  switch (vt) {
    case ViewType::TABLE: {
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
      break;
    }
    case ViewType::LIST: {
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
      break;
    }
    case ViewType::TREE: {
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
      break;
    }
    case ViewType::SEARCH: {
      QModelIndexList srcInds;
      for (const auto& proInd : m_advanceSearchView->selectionModel()->selectedRows()) {
        const auto& ind = m_proxyModel->mapToSource(proInd);
        srcInds.append(ind);
        filePaths.append(m_searchSrcModel->filePath(ind));
        urls.append(QUrl::fromLocalFile(filePaths.back()));
      }
      if (dropAct == Qt::CopyAction) {
        m_searchSrcModel->CopiedSomething(srcInds);
      } else if (dropAct == Qt::MoveAction) {
        m_searchSrcModel->CutSomething(srcInds);
      }
      break;
    }
    case ViewType::SCENE: {
      qDebug("Todo getFilePathsAndUrls");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        filePaths.append(m_dbModel->filePath(ind));
        urls.append(QUrl::fromLocalFile(filePaths.back()));
      }
      break;
    }
    default: {
      qDebug("No getFilePathsAndUrls");
    }
  }
  return {filePaths, urls};
}

std::pair<QStringList, QStringList> ContentPanel::getFilePrepathsAndName(const bool isSearchRecycle) const {
  QStringList prepaths;
  QStringList names;
  prepaths.reserve(10);
  names.reserve(10);
  auto vt = GetCurViewType();
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
        const auto& ind = m_proxyModel->mapToSource(proInd);
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
      qDebug("Todo getFilePrepathsAndName");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        prepaths.append(m_dbModel->absolutePath(ind));
        names.append(m_dbModel->fileName(ind));
      }
      break;
    }
    default: {
      qDebug("No getFilePrepathsAndName");
      break;
    }
  }

  if (prepaths.size() != names.size()) {
    qWarning("getFilePrepathsAndName size differ");
    return {};
  }
  return {prepaths, names};
}

int ContentPanel::getSelectedRowsCount() const {
  auto vt = GetCurViewType();
  switch (vt) {
    case ViewType::TABLE: {
      return m_fsTableView->selectionModel()->selectedRows().size();
    }
    case ViewType::LIST: {
      return m_fsListView->selectionModel()->selectedRows().size();
    }
    case ViewType::TREE: {
      return m_fsTreeView->selectionModel()->selectedRows().size();
    }
    case ViewType::SEARCH: {
      return m_advanceSearchView->selectionModel()->selectedRows().size();
    }
    case ViewType::SCENE: {
      return m_sceneTableView->selectionModel()->selectedIndexes().size();
    }
    case ViewType::MOVIE: {
      return m_movieView->selectionModel()->selectedRows().size();
    }
    default: {
      qDebug("No getSelectedRowsCount");
    }
  }
  return -1;
}

QString ContentPanel::getCurFilePath() const {
  auto vt = GetCurViewType();
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
      return m_searchSrcModel->filePath(m_proxyModel->mapToSource(m_advanceSearchView->currentIndex()));
    }
    case ViewType::SCENE: {
      return m_scenesModel->filePath(m_sceneTableView->currentIndex());
    }
    case ViewType::MOVIE: {
      return m_dbModel->filePath(m_movieView->currentIndex());
    }
    default: {
      qDebug("No getCurFilePath");
    }
  }
  return "";
}

QFileInfo ContentPanel::getFileInfo(const QModelIndex& ind) const {
  auto vt = GetCurViewType();
  switch (vt) {
    case ViewType::TABLE: {
      return m_fsModel->fileInfo(ind);
    }
    case ViewType::LIST: {
      return m_fsModel->fileInfo(ind);
    }
    case ViewType::TREE: {
      return m_fsModel->fileInfo(ind);
    }
    case ViewType::SEARCH: {
      return m_searchSrcModel->fileInfo(m_proxyModel->mapToSource(ind));
    }
    case ViewType::SCENE: {
      return m_scenesModel->fileInfo(ind);
    }
    case ViewType::MOVIE: {
      return m_dbModel->fileInfo(ind);
    }
    default: {
      qDebug("No getFileInfo");
      break;
    }
  }
  return {};
}
