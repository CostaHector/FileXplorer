#include "FloatingModels.h"
#include "PathTool.h"
#include "FileTool.h"
#include "StringTool.h"

#include <QPixmap>
#include <QFileIconProvider>
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
    QPixmap pm;
    const QString imgKey = StringTool::PathJoinPixmapSize(mDataLst[rw], mWidth, mHeight);
    if (mPixCache.find(imgKey, &pm)) {
      return pm;
    }
    if (QFile{mDataLst[rw]}.size() > 10 * 1024 * 1024) { // 10MB
      return {};                        // files too large
    }
    if (!pm.load(mDataLst[rw])) {
      return {}; // load failed
    }
    if (pm.width() * mHeight >= pm.height() * mWidth) {
      pm = pm.scaledToWidth(mWidth, Qt::FastTransformation);
    } else {
      pm = pm.scaledToHeight(mHeight, Qt::FastTransformation);
    }
    mPixCache.insert(imgKey, pm);
    return pm;
  }
  return {};
}

void ImgsModel::onIconSizeChange(const QSize& newSize) {
  if (newSize.width() == mWidth && newSize.height() == mHeight) {
    return;
  }
  mWidth = newSize.width();
  mHeight = newSize.height();
  mPixCache.clear();
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
    static QFileIconProvider ip;
    return ip.icon(QFileInfo{mDataLst[rw]});
  }
  return {};
}

// ----------------
