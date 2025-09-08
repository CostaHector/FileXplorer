#include "DevicesDriveModel.h"
#include "TableFields.h"
#include "DisplayEnhancement.h"
#include "Logger.h"
#include <QApplication>
#include <QStyle>

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
  if (role == Qt::DecorationRole) {
    if (col == FIELD_E::ROOT_PATH) {
      static const QIcon driveIcon = QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DriveHDIcon);
      return driveIcon;
    }
  } else if (role == Qt::DisplayRole) {
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

QString DevicesDriveModel::GetRootPath(const QModelIndex& idx) const {
  const QModelIndex& shiftedRootPathIndex = idx.siblingAtColumn(DEV_DRV_TABLE::ROOT_PATH);
  if (!shiftedRootPathIndex.isValid()) {
    LOG_W("shiftedGuidIndex invalid");
    return {};
  }
  return QSqlTableModel::data(shiftedRootPathIndex).toString();
}

QString DevicesDriveModel::GetGuid(const QModelIndex& idx) const {
  const QModelIndex& shiftedGuidIndex = idx.siblingAtColumn(DEV_DRV_TABLE::GUID);
  if (!shiftedGuidIndex.isValid()) {
    LOG_W("shiftedGuidIndex invalid");
    return {};
  }
  return QSqlTableModel::data(shiftedGuidIndex).toString();
}

QString DevicesDriveModel::GetMountedPoint(const QModelIndex& idx) const {
  const QModelIndex& shiftedMountPntIndex = idx.siblingAtColumn(DEV_DRV_TABLE::MOUNT_POINT);
  if (!shiftedMountPntIndex.isValid()) {
    LOG_W("guidIndex invalid");
    return {};
  }
  return QSqlTableModel::data(shiftedMountPntIndex).toString();
}
