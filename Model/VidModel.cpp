#include "VidModel.h"
#include "PublicVariable.h"

#include <QBrush>
#include <QDirIterator>
#include <QFile>

VidModel::VidModel(QObject* parent) : DifferRootFileSystemModel{parent} {}

int VidModel::appendAPath(const QString& path) {
  decltype(m_vids) deltaVids;
  QDirIterator it(path, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    deltaVids.append(it.filePath());
  }
  if (deltaVids.isEmpty()) {
    return 0;
  }
  beginInsertRows(QModelIndex(), m_vids.size(), m_vids.size() + deltaVids.size() - 1);
  m_vids += deltaVids;
  endInsertRows();
  return deltaVids.size();
}

int VidModel::appendRows(const QStringList& lst) {
  if (lst.isEmpty()) {
    return 0;
  }
  beginInsertRows(QModelIndex(), m_vids.size(), m_vids.size() + lst.size() - 1);
  m_vids += lst;
  endInsertRows();
  return lst.size();
}

QVariant VidModel::data(const QModelIndex& index, int role) const {
  if (not index.isValid()) {
    return QVariant();
  }
  if (not(0 <= index.row() and index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    const int section = index.column();
    switch (section) {
      case 0:
        return m_vids[index.row()];
      default:
        return QVariant();
    }
  } else if (role == Qt::ItemDataRole::ForegroundRole) {
    if (not QFile::exists(m_vids[index.row()])) {
      return QBrush(Qt::GlobalColor::red);
    }
    return QBrush(Qt::GlobalColor::black);
  } else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignLeft);
  }
  return QVariant();
}

bool VidModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::DisplayRole and index.column() == 0) {  // 0: value
    m_vids[index.row()] = value.toString();
    // check is value valid. if valid write into Preference setting;
    emit dataChanged(index, index, {Qt::DisplayRole});
  }
  return QAbstractItemModel::setData(index, value, role);
}

void VidModel::clear() {
  beginRemoveRows(QModelIndex(), 0, m_vids.size() - 1);
  m_vids.clear();
  endRemoveRows();
}

void VidModel::whenFilesDeleted(const QItemSelection& selections) {
  for (const QItemSelectionRange& rng : selections) {
    emit dataChanged(rng.topLeft(), rng.bottomRight(), {Qt::ForegroundRole});
  }
}
void VidModel::updatePlayableForeground() {
  if (rowCount() != 0) {
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::ForegroundRole});
  }
}

int VidModel::getNextAvailableVidUrl(const QUrl& startFrom, const QModelIndexList& notAvailList) const {
  const QString& startStr = startFrom.toLocalFile();
  const int startRow = m_vids.indexOf(startStr);
  if (startRow == -1) {
    qWarning("Search startFrom url[%s] not found", qPrintable(startStr));
    return -1;
  }
  for (int row = startRow; row < rowCount(); ++row) {
    auto ind = index(row, 0);
    if (notAvailList.contains(ind)) {
      continue;
    }
    if (not QFileInfo(m_vids[row]).isFile()) {
      continue;
    }
    return row;
  }
  qDebug("Available url not found during index[%d, %d)", startRow, rowCount());
  return -1;
}

QStringList VidModel::getToRemoveFileList(const QModelIndexList& toRmvList) const {
  decltype(getToRemoveFileList({})) ans;
  for (const auto& ind : toRmvList) {
    const QString& pth = filePath(ind);
    if (not QFileInfo(pth).isFile()) {
      continue;
    }
    ans.append(pth);
  }
  return ans;
}
