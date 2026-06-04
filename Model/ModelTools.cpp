#include "ModelTools.h"
#include "Logger.h"

namespace ModelTools {

QList<int> GetSortedUniqueRowsFromIndexes(const QModelIndexList& indexes) {
  QList<int> sortedAndUnqiueRows;
  sortedAndUnqiueRows.reserve(indexes.size());
  for (const QModelIndex& index : indexes) {
    if (!index.isValid()) {
      LOG_W("index(%d, %d) invalid", index.row(), index.column());
      continue;
    }
    int row = index.row();
    sortedAndUnqiueRows.append(row);
  }
  if (sortedAndUnqiueRows.isEmpty()) {
    LOG_W("No valid rows to remove");
    return {};
  }
  std::sort(sortedAndUnqiueRows.begin(), sortedAndUnqiueRows.end());
  auto firstDuplicateIt = std::unique(sortedAndUnqiueRows.begin(), sortedAndUnqiueRows.end());
  sortedAndUnqiueRows.erase(firstDuplicateIt, sortedAndUnqiueRows.end());
  return sortedAndUnqiueRows;
}

QList<FRONT_BACK_ROW_NUMBER_PAIR> MergeList2SectionsRange(const QList<int>& sortedAndUnqiueRows) {
  if (sortedAndUnqiueRows.isEmpty()) {
    return {};
  }
  QList<FRONT_BACK_ROW_NUMBER_PAIR> ranges;
  int start = sortedAndUnqiueRows.first();
  int end = start;
  for (int i = 1; i < sortedAndUnqiueRows.size(); ++i) {
    if (sortedAndUnqiueRows[i] == end + 1) {
      // 连续行，扩展范围
      end = sortedAndUnqiueRows[i];
    } else {
      // 不连续，保存当前范围并开始新范围
      ranges.push_back(std::make_pair(start, end));
      start = sortedAndUnqiueRows[i];
      end = start;
    }
  }
  // 添加最后一个范围
  ranges.append(std::make_pair(start, end));
  return ranges;
}

}  // namespace ModelTools
