#ifndef SQLTABLETESTPRECONDITIONTOOL_H
#define SQLTABLETESTPRECONDITIONTOOL_H
#include <QSqlTableModel>
#include "TDir.h"

namespace SqlTableTestPreconditionTool {
bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QStringList expectsList, Qt::ItemDataRole role=Qt::DisplayRole);
bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QList<int> expectsList, Qt::ItemDataRole role=Qt::DisplayRole);
QModelIndexList GetIndexessAtOneRow(const QSqlTableModel& model, int rowBegin, int rowEnd, int column);

bool CreateFileStructure(TDir& tDir);
bool CreateFileStructurePsonFiles(TDir& tDir);
}

#endif  // SQLTABLETESTPRECONDITIONTOOL_H
