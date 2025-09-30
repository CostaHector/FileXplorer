#include "SqlTableTestPreconditionTool.h"
#include "Logger.h"
#include <QDebug>

namespace SqlTableTestPreconditionTool {
bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QStringList expectsList, Qt::ItemDataRole role) {
  QStringList actualsList;
  for (const QModelIndex& ind : indexes) {
    actualsList.push_back(model.data(ind, role).toString());
  }
  std::sort(actualsList.begin(), actualsList.end());
  std::sort(expectsList.begin(), expectsList.end());
  int n1 = actualsList.size(), n2 = expectsList.size();
  if (actualsList == expectsList) {
    return true;
  }
  if (n1 != n2) {
    LOG_W("list length[%d, %d] not equal", n1, n2);
    return false;
  }

  qDebug() << "expects:" << expectsList;
  qDebug() << "actuals:" << actualsList;
  return false;
}

bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QList<int> expectsList, Qt::ItemDataRole role) {
  QList<int> actualsList;
  for (const QModelIndex& ind : indexes) {
    actualsList.push_back(model.data(ind, role).toInt());
  }
  std::sort(actualsList.begin(), actualsList.end());
  std::sort(expectsList.begin(), expectsList.end());
  int n1 = actualsList.size(), n2 = expectsList.size();
  if (actualsList == expectsList) {
    return true;
  }
  if (n1 != n2) {
    LOG_W("list length[%d, %d] not equal", n1, n2);
    return false;
  }

  qDebug() << "expects:" << expectsList;
  qDebug() << "actuals:" << actualsList;
  return false;
}

QModelIndexList GetIndexessAtOneRow(const QSqlTableModel& model, int rowBegin, int rowEnd, int column) {
  if (column < 0 || column >= model.columnCount()) {
    LOG_E("column[%d] out of range [0, %d)", column, model.columnCount());
    return {};
  }
  if (rowBegin > rowEnd || (rowBegin < 0) || (rowEnd > model.rowCount())) {
    LOG_E("row range invalid: must [beg:%d <= end:%d] an in [0, %d)", rowBegin, rowEnd, 0, model.rowCount());
    return {};
  }

  QModelIndexList indexesNames;
  indexesNames.reserve(rowEnd - rowBegin);
  for (int r = rowBegin; r < rowEnd; ++r) {
    indexesNames.push_back(model.index(r, column));
  }
  return indexesNames;
}
}  // namespace SqlTableTestPreconditionTool
