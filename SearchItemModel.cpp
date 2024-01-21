#include "SearchItemModel.h"
#include <QDateTime>
#include <QDirIterator>

QString DirFilter2String(const QDir::Filters& dirFilters) {
  QStringList dirFilterStrList;
  if (dirFilters & QDir::Filter::Files) {
    dirFilterStrList << "Files";
  }
  if (dirFilters & QDir::Filter::Dirs) {
    dirFilterStrList << "Dirs";
  }
  return dirFilterStrList.join('|');
}

QDir::Filters String2DirFilters(const QString& dirFilterStr) {
  if (dirFilterStr.isEmpty()) {
    return QDir::Filter::Files | QDir::Filter::Dirs;
  }
  QDir::Filters dirFilters{};
  for (const QString& filter : dirFilterStr.split('|')) {
    if (filter == "Files") {
      dirFilters |= QDir::Filter::Files;
    } else if (filter == "Dirs") {
      dirFilters |= QDir::Filter::Dirs;
    } else {
      qDebug("Filter Type[%s] not supported", qPrintable(filter));
    }
  }
  return dirFilters;
}
SearchItemModel::SearchItemModel(QObject* parent) : QStandardItemModel{parent} {}

SearchItemModel::~SearchItemModel() {
  decltype(indexData_) tempForRelease;
  indexData_.swap(tempForRelease);
}

bool SearchItemModel::ChangeRootPathOrFilter(const QString& rootPath,
                                             const QDir::Filters& dirFilters,
                                             const QStringList& fileTypeFilter,
                                             bool forceUpdate) {
  if (m_Key == rootPath + '$' + DirFilter2String(dirFilters) + '$' + fileTypeFilter.join('|')) {
    qDebug("skip");
    return true;
  }
  m_rootPath = rootPath;
  m_Key = rootPath + '$' + DirFilter2String(dirFilters) + '$' + fileTypeFilter.join('|');
  qDebug("Key: %s", qPrintable(m_Key));

  setRowCount(0);
  auto showTable = [this]() {
    for (const FileProperty& fp : indexData_[m_Key]) {
      appendRow({new QStandardItem(fp.name), new QStandardItem(QString::number(fp.size)), new QStandardItem(fp.type),
                 new QStandardItem(QString::number(fp.createdTime)), new QStandardItem(fp.relPath)});
    }
  };

  // not contains => init and rec
  // contain and force => clear and rec
  // contain and not force => direct show
  const bool needRecusive = not indexData_.contains(m_Key) or forceUpdate;
  if (not needRecusive) {
    showTable();
    return true;
  }
  if (indexData_.contains(m_Key)) {
    qDebug("Met Key: %s", qPrintable(m_Key));
    QList<FileProperty> tempListForRelease;
    const int beforeItemsCount = indexData_[m_Key].size();
    tempListForRelease.reserve(beforeItemsCount);
    indexData_[m_Key].swap(tempListForRelease);
  } else {
    qDebug("Unmet Key: %s", qPrintable(m_Key));
    indexData_[m_Key] = {};
  }

  const int PRE_PATH_LEN = rootPath.size();
  QDirIterator it(rootPath, fileTypeFilter, dirFilters | QDir::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QFileInfo& fi = it.fileInfo();
    const QString& relPath = fi.absoluteFilePath().mid(PRE_PATH_LEN + 1);
    FileProperty fp;
    strcpy_s(fp.name, sizeof(fp.name), fi.fileName().toStdString().c_str());
    fp.size = fi.size();
    strcpy_s(fp.type, sizeof(fp.type), fi.suffix().toStdString().c_str());
    fp.createdTime = fi.birthTime().toMSecsSinceEpoch();
    strcpy_s(fp.relPath, sizeof(fp.relPath), relPath.toStdString().c_str());

    indexData_[m_Key] << fp;
  }
  showTable();
  qDebug("Search out %d items[%s]", indexData_[m_Key].size(), qPrintable(m_Key));
  return true;
}

QVariant SearchItemModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    return QStandardItemModel::data(index, role);
  }
  if (index.column() == SearchKey::NAME_COLUMN and role == Qt::DecorationRole) {
    return m_icon_provider.icon(QStandardItemModel::data(index, Qt::DisplayRole).toString());
  }
  return {};

  if (!index.isValid() || role != Qt::DisplayRole) {
    qDebug("Invalid index");
    return {};
  }
  if (not indexData_.contains(m_Key)) {
    qDebug("Key[%s] not in lib", qPrintable(m_Key));
    return {};
  }
  const int r = index.row(), c = index.column();
  const FileProperty& filePro = indexData_[m_Key][r];
  switch (c) {
    case 0:
      return filePro.name;
    case 1:
      return filePro.size;
    case 2:
      return filePro.type;
    case 3:
      return filePro.createdTime;
    case 4:
      return filePro.relPath;
    default:
      qDebug("Column[%d] not allowable", c);
      return {};
  }
}

Qt::ItemFlags SearchItemModel::flags(const QModelIndex& index) const {
  if (index.isValid()) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  return {};
}
