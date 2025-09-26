#include "RedundantImageModel.h"
#include "PathTool.h"
#include "DataFormatter.h"
#include <QFile>
#include <QPixmap>

using namespace DuplicateImageMetaInfo;

QVariant RedundantImageModel::data(const QModelIndex& index, int role) const {
  if (m_paf == nullptr || !index.isValid()) {
    return {};
  }
  const REDUNDANT_IMG_INFO& item = m_paf->operator[](index.row());
  switch (role) {
    case Qt::DisplayRole: {
      switch (index.column()) {
#define DUP_IMAGE_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) \
  case DuplicateImageMetaInfo::enu: \
    return formatter(item.m_##enu);     //
        DUP_IMAGE_META_INFO_KEY_MAPPING //
#undef DUP_IMAGE_META_INFO_KEY_ITEM     //
            default : return {};
      }
      break;
    }
    case Qt::DecorationRole: {
      if (index.column() == DI_TABLE_HEADERS_COUNT - 1) {
        if (!QFile::exists(item.m_AbsPath)) {
          return {}; // not exist
        }
        return QPixmap{item.m_AbsPath}.scaledToWidth(128);
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
    if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
      return DI_TABLE_HEADERS[section];
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
    LOG_W("r[%d] out of range[0, %d)", r, rowCount());
    return "";
  }
  return m_paf->operator[](r).m_AbsPath;
}

int RedundantImageModel::setRootPath(const RedundantImagesList* p_af) {
  int beforeRow = rowCount();
  int afterRow = p_af != nullptr ? p_af->size() : 0;
  LOG_D("setRootPath. RowCountChanged: %d->%d", beforeRow, afterRow);

  RowsCountBeginChange(beforeRow, afterRow);
  m_paf = p_af;
  RowsCountEndChange();
  return afterRow;
}
