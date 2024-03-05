#include "MySearchModel.h"

const QStringList MySearchModel::HORIZONTAL_HEADER_NAMES = {"name", "size", "type", "date", "relative path"};

auto MySearchModel::_updatePlanetList() -> void {
  if (m_rootPath.isEmpty()) {
    qDebug("reject do under path \"\"");
    return;
  }
  if (not QFileInfo(m_rootPath).isDir()) {
    return;
  }

  //  emit dataChanged(index(0, 0), index(r, c), {Qt::DisplayRole, Qt::DecorationRole});
  this->beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
  m_planetList.clear();
  this->endRemoveRows();

  decltype(m_planetList) newPlanetList;
  const int N = m_rootPath.size();
  QDirIterator it(m_rootPath, m_filters, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    QFileInfo fi = it.fileInfo();
    newPlanetList.append(FileProperty{fi.fileName(), fi.size(), fi.suffix(), fi.lastModified(), fi.absoluteFilePath().mid(N)});
  }
  qDebug("> %d item(s) | %d | enter [%s] ", newPlanetList.size(), int(m_filters), qPrintable(m_rootPath));
  this->beginInsertRows(QModelIndex(), 0, newPlanetList.size() - 1);
  newPlanetList.swap(m_planetList);
  this->endInsertRows();
}

auto MySearchModel::checkPathNeed(const QString& path) const -> bool {
  if (path.isEmpty()) {
    return false;
  }
  if (not QFileInfo(path).isDir()) {
    return false;
  }
  return true;
}

auto MySearchModel::setRootPath(const QString& path) -> void {
  if (m_rootPath == path) {
    return;
  }
  if (not checkPathNeed(path)) {
    return;
  }
  m_rootPath = path;
  _updatePlanetList();
  qDebug() << "MySearchModel::setRootPath";
}

auto MySearchModel::setFilter(QDir::Filters filters) -> void {
  if (m_filters == filters) {
    return;
  }
  m_filters = filters;
  if (not checkPathNeed(m_rootPath)) {
    return;
  }
  _updatePlanetList();
  qDebug() << "MySearchModel::setFilter";
}

void MySearchModel::setRootPathAndFilter(const QString& path, QDir::Filters filters) {
  if (m_rootPath == path and m_filters == filters) {
    return;
  }
  if (not checkPathNeed(path)) {
    return;
  }
  m_rootPath = path;
  m_filters = filters;
  _updatePlanetList();
  qDebug() << "MySearchModel::setRootPathAndFilter";
}

QVariant MySearchModel::data(const QModelIndex& index, int role) const {
  if (not index.isValid()) {
    return QVariant();
  }
  if (not(0 <= index.row() and index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0:
        return m_planetList[index.row()].name;
      case 1:
        return m_planetList[index.row()].size;
      case 2:
        return m_planetList[index.row()].type;
      case 3:
        return m_planetList[index.row()].modifiedDate;
      case 4:
        return m_planetList[index.row()].relPath;
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 0) {
      return m_iconProvider.icon(QFileInfo(m_rootPath + m_planetList[index.row()].relPath));
    }
    return QVariant();
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 1) {
      // Todo  | Qt::AlignVCenter
      return Qt::AlignRight;
    }
    return int(Qt::AlignLeft | Qt::AlignTop);
  }
  return QVariant();
}

auto MySearchModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
  if (role != Qt::DisplayRole) {
    return QVariant();
  }
  if (orientation == Qt::Orientation::Horizontal) {
    return HORIZONTAL_HEADER_NAMES[section];
  }
  return section + 1;
}
