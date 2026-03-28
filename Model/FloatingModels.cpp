#include "FloatingModels.h"
#include "PathTool.h"
#include "FileTool.h"
#include "StringTool.h"

#include <QDir>

constexpr int FloatingModels::BATCH_LOAD_COUNT;

QVariant FloatingModels::data(const QModelIndex& index, int role) const {
  const int rw = index.row();
  if (isOuterBound(rw)) {
    return {};
  }
  if (role == Qt::DisplayRole) {
    return mDataLst[rw];
  }
  return {};
}

int FloatingModels::UpdateData(const QStringList& newDataLst) { // dont use newImgsLst after
  setRootPath(PathTool::longestCommonPrefix(newDataLst));

  RowsCountBeginChange(mDataLst.size(), newDataLst.size());

  mDataLst = newDataLst;
  m_curLoadedCount = mDataLst.size();

  RowsCountEndChange();
  return mDataLst.size();
}

QString FloatingModels::filePath(const QModelIndex& index) const {
  const int rw = index.row();
  if (isOuterBound(rw)) {
    return {};
  }
  return mDataLst[rw];
}

int FloatingModels::setDirPath(const QString& path, const QStringList& sFilters, bool loadAllIn1Time) {
  QDir dir{path, "", QDir::SortFlag::Name, QDir::Filter::Files};
  dir.setNameFilters(sFilters);
  QStringList datas;
  foreach (QString name, dir.entryList()) {
    datas.append(path + '/' + name);
  }
  StringTool::ImgsSortNameLengthFirst(datas);
  if (loadAllIn1Time) {
    UpdateData(datas);
  } else {
    beginResetModel();
    mDataLst.swap(datas);
    m_curLoadedCount = 0;
    endResetModel();
  }
  return mDataLst.size();
}

bool FloatingModels::canFetchMore(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return false;
  }
  return m_curLoadedCount < mDataLst.size();
}

void FloatingModels::fetchMore(const QModelIndex& parent) {
  if (parent.isValid()) {
    return;
  }
  const int remainder = mDataLst.size() - m_curLoadedCount;
  const int fetchCntThisTime = qMin(BATCH_LOAD_COUNT, remainder);
  if (fetchCntThisTime <= 0) {
    return;
  }
  beginInsertRows(QModelIndex(), m_curLoadedCount, m_curLoadedCount + fetchCntThisTime - 1);
  m_curLoadedCount += fetchCntThisTime;
  endInsertRows();
}

QStringList FloatingModels::rel2fileNames(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) {
    return {};
  }
  QStringList relativePaths2FileName;
  relativePaths2FileName.reserve(indexes.size());
  const int N = rootPath().size();
  for (const QModelIndex& index : indexes) {
    const QString& fullPath = filePath(index);
    relativePaths2FileName.push_back(fullPath.isEmpty() ? "" : fullPath.mid(N + 1));
  }
  return relativePaths2FileName;
}

int FloatingModels::AfterRowsRemoved(const QModelIndexList& indexes) {
  if (indexes.isEmpty()) {
    return {};
  }
  const auto rowElementsRmv = [this](int beg, int end) { mDataLst.erase(mDataLst.begin() + beg, mDataLst.begin() + end); };
  int rmvedCnt = onRowsRemoved(indexes, rowElementsRmv);
  m_curLoadedCount = mDataLst.size();
  return rmvedCnt;
}

// ------------------
QVariant ImgsModel::data(const QModelIndex& index, int role) const {
  const int rw = index.row();
  if (isOuterBound(rw)) {
    return {};
  }
  if (role == Qt::DecorationRole) {
    return GetDecorationPixmap(mDataLst[rw]);
  } else if (role == Qt::ToolTipRole) {
    return mDataLst[rw];
  }
  return {};
}

int ImgsModel::UpdateData(const QStringList& newDataLst) {
  setRootPath(PathTool::longestCommonPrefix(newDataLst));

  beginResetModel();

  mDataLst = newDataLst;
  m_curLoadedCount = mDataLst.size();

  endResetModel();
  return mDataLst.size();
}

// ----------------

QVariant VidsModel::data(const QModelIndex& index, int role) const {
  const int rw = index.row();
  if (isOuterBound(rw)) {
    return {};
  }
  if (role == Qt::DisplayRole) {
    return PathTool::GetBaseName(mDataLst[rw]);
  } else if (role == Qt::DecorationRole) {
    return GetDecorationPixmap(mDataLst[rw]);
  }
  return {};
}

// ----------------

QVariant OthersModel::data(const QModelIndex& index, int role) const {
  const int rw = index.row();
  if (isOuterBound(rw)) {
    return {};
  }
  if (role == Qt::DisplayRole) {
    return PathTool::fileName(mDataLst[rw]);
  } else if (role == Qt::DecorationRole) {
    return GetDecorationPixmap(mDataLst[rw]);
  }
  return {};
}
