#ifndef SQLTABLETESTPRECONDITIONTOOL_H
#define SQLTABLETESTPRECONDITIONTOOL_H
#include <QSqlTableModel>

namespace SqlTableTestPreconditionTool {
bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QStringList expectsList, Qt::ItemDataRole role=Qt::DisplayRole);
bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QList<int> expectsList, Qt::ItemDataRole role=Qt::DisplayRole);
QModelIndexList GetIndexessAtOneRow(const QSqlTableModel& model, int rowBegin, int rowEnd, int column);
}

#endif  // SQLTABLETESTPRECONDITIONTOOL_H
