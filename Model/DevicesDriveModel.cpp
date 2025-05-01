#include "DevicesDriveModel.h"
#include "Tools/FileDescriptor/DevicesAndDriverDb.h"
#include "public/DisplayEnhancement.h"
using namespace DEV_DRV_TABLE;
DevicesDriveModel::DevicesDriveModel(QObject* parent, QSqlDatabase con)  //
    : QSqlTableModel{parent, con} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}

QVariant DevicesDriveModel::data(const QModelIndex& idx, int role) const {
  if (!idx.isValid()) {
    return QVariant();
  }
  const int col = idx.column();
  if (role == Qt::DisplayRole) {
    if (col == FIELD_E::TOTAL_BYTES || col == FIELD_E::AVAIL_BYTES) {
      const qint64& sz = QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong();
      return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(sz);
    }
  }
  return QSqlTableModel::data(idx, role);
}

QVariant DevicesDriveModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Vertical) {
    if (role == Qt::TextAlignmentRole) {
      return Qt::AlignRight;
    }
  }
  return QSqlTableModel::headerData(section, orientation, role);
}
