#include "MyQSqlTableModel.h"

#include "PublicVariable.h"

MyQSqlTableModel::MyQSqlTableModel(QObject* parent, QSqlDatabase con) : QSqlTableModel{parent, con} {
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}

QVariant MyQSqlTableModel::data(const QModelIndex& idx, int role) const {
  if (not idx.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    if (idx.column() == DB_HEADER_KEY::DB_SIZE_COLUMN) {
      const auto total = QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toUInt();
      const qint64 xGiB = total / (1 << 30);
      const qint64 xMiB = total % (1 << 30) / (1 << 20);
      const qint64 xkiB = total % (1 << 30) % (1 << 20) / (1 << 10);
      const qint64 xB = total % (1 << 30) % (1 << 20) % (1 << 10);
      return QString("%1'%2'%3'%4").arg(xGiB).arg(xMiB).arg(xkiB).arg(xB);
    }
  }
  return QSqlTableModel::data(idx, role);
}
