#include "FloatingModels.h"
#include "PublicVariable.h"
#include "Tools/PathTool.h"
#include <QPixmap>
#include <QFileIconProvider>
#include <QDir>

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

void FloatingModels::UpdateData(const QStringList& newDataLst) {  // dont use newImgsLst after
  RowsCountBeginChange(mDataLst.size(), newDataLst.size());
  mDataLst = newDataLst;
  m_curLoadedCount = mDataLst.size();
  RowsCountEndChange();
}

QString FloatingModels::filePath(const QModelIndex& index) const {
  const int rw = index.row();
  if (rw < 0 || rw >= mDataLst.size()) {
    return {};
  }
  return mDataLst[rw];
}
// ------------------

constexpr int ImgsModel::BATCH_LOAD_COUNT;
QVariant ImgsModel::data(const QModelIndex& index, int role) const {
  const int rw = index.row();
  if (isOuterBound(rw)) {
    return {};
  }
  if (role == Qt::DecorationRole) {
    const QPixmap pm{mDataLst[rw]};
    // w/h > 480/280 = 48 / 28 = 12 / 7
    if (pm.width() * 7 >= pm.height() * 12) {
      return pm.scaledToWidth(480);
    }
    return pm.scaledToHeight(280);
  }
  return {};
}

void ImgsModel::setDirPath(const QString& path) {
  beginResetModel();
  mDataLst.clear();
  QDir dir(path, "", QDir::SortFlag::Name, QDir::Filter::Files);
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  for (const QString& name : dir.entryList()) {
    mDataLst.append(path + '/' + name);
  }
  m_curLoadedCount = 0;
  endResetModel();
}

bool ImgsModel::canFetchMore(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return false;
  }
  return m_curLoadedCount < mDataLst.size();
}

void ImgsModel::fetchMore(const QModelIndex& parent) {
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
// ----------------
QVariant VidsModel::data(const QModelIndex& index, int role) const {
  const int rw = index.row();
  if (isOuterBound(rw)) {
    return {};
  }
  if (role == Qt::DisplayRole) {
    return PATHTOOL::GetBaseName(mDataLst[rw]);
  } else if (role == Qt::DecorationRole) {
    static QFileIconProvider ip;
    return ip.icon(mDataLst[rw]);
  }
  return {};
}

// ----------------
