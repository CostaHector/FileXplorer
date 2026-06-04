#ifndef MODELTOOLS_H
#define MODELTOOLS_H

#include <QList>
#include <QModelIndexList>

namespace ModelTools {
using FuncRangeRemoveCallback = std::function<void(int, int)>;
using FuncRangeListRemoveCallback = std::function<void(const QList<std::pair<int, int>>&)>;
QList<int> GetSortedUniqueRowsFromIndexes(const QModelIndexList& indexes);

using FRONT_BACK_ROW_NUMBER_PAIR = std::pair<int, int>;
QList<FRONT_BACK_ROW_NUMBER_PAIR> MergeList2SectionsRange(const QList<int>& sortedAndUnqiueRows);
}

#endif  // MODELTOOLS_H
