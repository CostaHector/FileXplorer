#include "MyQSqlTableModel.h"
#include "public/DisplayEnhancement.h"
#include <QSqlQuery>

MyQSqlTableModel::MyQSqlTableModel(QObject* parent, QSqlDatabase con)  //
    : QSqlTableModel{parent, con} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}

QVariant MyQSqlTableModel::data(const QModelIndex& idx, int role) const {
  if (!idx.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    if (idx.column() == DB_HEADER_KEY::Size)
      return FILE_PROPERTY_DSP::sizeToHumanReadFriendly(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toLongLong());
  }
  return QSqlTableModel::data(idx, role);
}
