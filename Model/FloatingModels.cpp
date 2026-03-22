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
  RowsCountBeginChange(mDataLst.size(), newDataLst.size());

  mDataLst = newDataLst;
  m_curLoadedCount = mDataLst.size();

  RowsCountEndChange();
  return mDataLst.size();
}

QString FloatingModels::filePath(const QModelIndex& index) const {
  const int rw = index.row();
  if (rw < 0 || rw >= mDataLst.size()) {
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
