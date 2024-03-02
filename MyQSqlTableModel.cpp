#include "MyQSqlTableModel.h"

MyQSqlTableModel::MyQSqlTableModel(QObject *parent, QSqlDatabase con)
    : QSqlTableModel{parent, con}
{
  setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
}
