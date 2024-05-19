#include "MyQSqlTableModel.h"

#include "PublicVariable.h"
#include "public/DisplayEnhancement.h"

MyQSqlTableModel::MyQSqlTableModel(QObject* parent, QSqlDatabase con) : QSqlTableModel{parent, con} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}

QVariant MyQSqlTableModel::data(const QModelIndex& idx, int role) const {
  if (not idx.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    if (idx.column() == DB_HEADER_KEY::DB_SIZE_COLUMN)
      return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toUInt());
  }
  return QSqlTableModel::data(idx, role);
}
