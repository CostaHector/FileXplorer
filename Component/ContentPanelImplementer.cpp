#include "ViewsStackedWidget.h"
#include "ScenesListModel.h"
#include "ViewTypeTool.h"

using namespace ViewTypeTool;

const char* ViewsStackedWidget::GetCurViewName() const {
  return ViewTypeTool::c_str(GetVt());
}

int ViewsStackedWidget::AddView(ViewType vt, QWidget* w) {
  return m_name2ViewIndex[vt] = addWidget(w);
}

QString ViewsStackedWidget::getRootPath() const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE:
    case ViewType::LIST:
    case ViewType::TREE:
    case ViewType::SEARCH:
    case ViewType::SCENE:
    case ViewType::JSON: {
      return m_fsModel->rootPath();
    }
    case ViewType::CAST:
      return m_castDbModel->rootPath();
    default:
      qWarning("No rootpath in ViewType[%d]", int(vt));
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
      qWarning("No FilePath in ViewType[%d]", int(vt));
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
      qWarning("No SelectedRows in ViewType[%d]", int(vt));
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
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedIndexes()) {
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
      qWarning("No SelectedRows in ViewType[%d]", int(vt));
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
      qDebug("Todo: getFullRecords is not supported in scene model now");
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
      qWarning("No getFullRecords ViewType:%d", (int)vt);
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
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedIndexes()) {
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
      qDebug("No getFilePaths ViewType:%d", (int)vt);
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
      for (const auto& ind : m_sceneTableView->selectionModel()->selectedIndexes()) {
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
      qDebug("No getFilePrepaths ViewType:%d", (int)vt);
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
      qDebug("Todo: need complement");
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
      qDebug("No getTheJpgFolderPaths");
    }
  }
  return prepaths;
}

std::pair<QStringList, QList<QUrl>> ViewsStackedWidget::getFilePathsAndUrls(const Qt::DropAction dropAct) const {
  QStringList filePaths;
  QList<QUrl> urls;

  const auto Fill = [&filePaths, &urls](FileSystemModel* fsModel, const QModelIndexList& indexes, const Qt::DropAction dropAct) {
    filePaths.reserve(indexes.size());
    urls.reserve(indexes.size());
    for (const auto& ind : indexes) {
      filePaths.append(fsModel->filePath(ind));
      urls.append(QUrl::fromLocalFile(filePaths.back()));
    }
    if (dropAct == Qt::CopyAction) {
      fsModel->CopiedSomething(indexes);
    } else if (dropAct == Qt::MoveAction) {
      fsModel->CutSomething(indexes);
    }
  };

  const auto FillInSearchModel = [&filePaths, &urls](
                                     AdvanceSearchModel* searchSrcModel, SearchProxyModel* searchProxyModel,
                                     const QModelIndexList& proIndexes, const Qt::DropAction dropAct) {

    QModelIndexList srcIndexes;
    srcIndexes.reserve(proIndexes.size());
    filePaths.reserve(proIndexes.size());
    urls.reserve(proIndexes.size());
    for (const auto& proInd : proIndexes) {
      const auto& srcind = searchProxyModel->mapToSource(proInd);
      srcIndexes.append(srcind);
      filePaths.append(searchSrcModel->filePath(srcind));
      urls.append(QUrl::fromLocalFile(filePaths.back()));
    }
    if (dropAct == Qt::CopyAction) {
      searchSrcModel->CopiedSomething(srcIndexes);
    } else if (dropAct == Qt::MoveAction) {
      searchSrcModel->CutSomething(srcIndexes);
    }
  };


  auto vt = GetVt();
  switch (vt) {
    case ViewType::LIST: {
      Fill(m_fsModel, m_fsListView->selectionModel()->selectedRows(), dropAct);
      break;
    }
    case ViewType::TABLE: {
      Fill(m_fsModel, m_fsTableView->selectionModel()->selectedRows(), dropAct);
      break;
    }
    case ViewType::TREE: {
      Fill(m_fsModel, m_fsTreeView->selectionModel()->selectedRows(), dropAct);
      break;
    }
    case ViewType::SEARCH: {
      FillInSearchModel(m_searchSrcModel, m_searchProxyModel, m_advanceSearchView->selectionModel()->selectedRows(), dropAct);
      break;
    }
    case ViewType::SCENE: {
      qDebug("Todo getFilePathsAndUrls");
      break;
    }
    case ViewType::MOVIE: {
      for (const auto& ind : m_movieView->selectionModel()->selectedRows()) {
        filePaths.append(m_movieDbModel->filePath(ind));
        urls.append(QUrl::fromLocalFile(filePaths.back()));
      }
      break;
    }
    default: {
      qWarning("No getFilePathsAndUrls ViewType:%d", (int)vt);
    }
  }
  return {filePaths, urls};
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
      qDebug("Todo getFilePrepathsAndName");
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

int ViewsStackedWidget::getSelectedRowsCount() const {
  auto vt = GetVt();
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
    case ViewType::JSON: {
      return m_jsonTableView->selectionModel()->selectedRows().size();
    }
    default: {
      qDebug("No getSelectedRowsCount");
    }
  }
  return -1;
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
      qWarning("No getCurFilePath");
    }
  }
  qWarning("nothing selected in viewType[%d]", (int)vt);
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
      qWarning("No getCurFileName");
    }
  }
  return "";
}

QFileInfo ViewsStackedWidget::getFileInfo(const QModelIndex& ind) const {
  auto vt = GetVt();
  switch (vt) {
    case ViewType::TABLE:
    case ViewType::LIST:
    case ViewType::TREE: {
      return m_fsModel->fileInfo(ind);
    }
    case ViewType::SEARCH: {
      return m_searchSrcModel->fileInfo(m_searchProxyModel->mapToSource(ind));
    }
    case ViewType::SCENE: {
      return m_scenesModel->fileInfo(ind);
    }
    case ViewType::JSON: {
      return m_jsonModel->fileInfo(m_jsonProxyModel->mapToSource(ind));
    }
    case ViewType::MOVIE: {
      return m_movieDbModel->fileInfo(ind);
    }
    default: {
      qWarning("No getFileInfo");
      break;
    }
  }
  return {};
}
