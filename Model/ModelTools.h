#ifndef MODELTOOLS_H
#define MODELTOOLS_H

#include <QList>
#include <QModelIndexList>

namespace ModelTools {
using FuncRangeRemoveCallback = std::function<void(int, int)>;
using FuncRangeListRemoveCallback = std::function<void(const QList<std::pair<int, int>>&)>;
QList<int> GetIndexesRows(const QModelIndexList& indexes);
QList<std::pair<int, int>> MergeList2SectionsRange(const QList<int>& rows);
}

#endif  // MODELTOOLS_H
