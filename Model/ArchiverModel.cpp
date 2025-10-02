#include "ArchiverModel.h"
#include "DataFormatter.h"

const QStringList ArchiverModel::ARCHIVE_HORIZONTAL_HEADER{"Name", "Compressed(B)", "Original(B)"};

QVariant ArchiverModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  int r = index.row();
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0:
        return m_paf.key(r);
      case 1:
        return DataFormatter::formatFileSizeGMKB(m_paf.beforeSize(r));
      case 2:
        return DataFormatter::formatFileSizeGMKB(m_paf.afterSize(r));
      default:
        return {};
    }
  }
  return {};
}

QVariant ArchiverModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
      return ARCHIVE_HORIZONTAL_HEADER[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

void ArchiverModel::setRootPath(const QString& qzPath) {
  beginResetModel();
  m_paf.ReadAchiveFile(qzPath);
  endResetModel();
}

QString ArchiverModel::GetRelativeName(int r) const {
  if (r < 0 || r >= rowCount()) {
    return "";
  }
  return m_paf.key(r);
}
const QByteArray& ArchiverModel::GetByteArrayData(int r) const {
  if (r < 0 || r >= rowCount()) {
    static const QByteArray EMPTY_BA;
    return EMPTY_BA;
  }
  return m_paf.value(r);
}
