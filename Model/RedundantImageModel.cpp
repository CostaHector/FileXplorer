#include "RedundantImageModel.h"
#include "public/PathTool.h"
#include "public/DisplayEnhancement.h"
#include <QPixmap>

const QStringList RedundantImageModel::HORIZONTAL_HEADER{"Name", "Size(B)", "MD5", "Preview", "absPath"};

QVariant RedundantImageModel::data(const QModelIndex& index, int role) const {
  if (m_paf == nullptr or not index.isValid()) {
    return {};
  }
  const REDUNDANT_IMG_INFO& item = m_paf->operator[](index.row());
  switch (role) {
    case Qt::DisplayRole: {
      switch (index.column()) {
        case 0:
          return PathTool::fileName(item.filePath);
        case 1:
          return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(item.size);
        case 2:
          return item.md5;
        case 4:
          return PathTool::absolutePath(item.filePath);
        default:
          return {};
      }
      break;
    }
    case Qt::DecorationRole: {
      if (index.column() == HORIZONTAL_HEADER.size() - 1) {
        return QPixmap{item.filePath}.scaledToWidth(128);
      }
      break;
    }
    default:
      break;
  }
  return {};
}

QVariant RedundantImageModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  } else if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return HORIZONTAL_HEADER[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QString RedundantImageModel::filePath(const QModelIndex& index) const {
  if (m_paf == nullptr || !index.isValid()) {
    return "";
  }
  const int r = index.row();
  if (r < 0 || r >= rowCount()) {
    qWarning("r[%d] out of range[0, %d)", r, rowCount());
    return "";
  }
  return m_paf->operator[](r).filePath;
}

void RedundantImageModel::setRootPath(const REDUNDANT_IMG_BUNCH* p_af) {
  int beforeRow = rowCount();
  int afterRow = p_af != nullptr ? p_af->size() : 0;
  qDebug("setRootPath. RowCountChanged: %d->%d", beforeRow, afterRow);

  RowsCountBeginChange(beforeRow, afterRow);
  m_paf = p_af;
  RowsCountEndChange();
}
