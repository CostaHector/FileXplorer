#ifndef SQLTABLETESTPRECONDITIONTOOL_H
#define SQLTABLETESTPRECONDITIONTOOL_H
#include <QSqlTableModel>
#include "TDir.h"

namespace SqlTableTestPreconditionTool {
bool CheckIndexesDisplayRoleIgnoreOrder(const QSqlTableModel& model,  //
                                        const QModelIndexList& indexes,
                                        QVariantList expectsList, Qt::ItemDataRole role=Qt::DisplayRole);
QModelIndexList GetIndexessAtOneRow(const QSqlTableModel& model, int rowBegin, int rowEnd, int column);

bool CreateFileStructure(TDir& tDir, const QString& rel2FileEndWithSlash);
bool CreateFileStructurePsonFiles(TDir& tDir, const QString& rel2FileEndWithSlash);
}

#endif  // SQLTABLETESTPRECONDITIONTOOL_H
