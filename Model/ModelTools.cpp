#include "ModelTools.h"
#include "Logger.h"

namespace ModelTools {

QList<int> GetIndexesRows(const QModelIndexList& indexes) {
  QList<int> validRows;
  validRows.reserve(indexes.size());
  for (const QModelIndex& index : indexes) {
    if (!index.isValid()) {
      LOG_W("index(%d, %d) invalid", index.row(), index.column());
      continue;
    }
    int row = index.row();
    validRows.append(row);
  }
  if (validRows.isEmpty()) {
    LOG_W("No valid rows to remove");
    return {};
  }
  std::sort(validRows.begin(), validRows.end());
  auto firstDuplicateIt = std::unique(validRows.begin(), validRows.end());
  validRows.erase(firstDuplicateIt, validRows.end());
  return validRows;
}

QList<std::pair<int, int>> MergeList2SectionsRange(const QList<int>& rows) {
  if (rows.isEmpty()) {
    return {};
  }
  QList<std::pair<int, int>> ranges;
  int start = rows.first();
  int end = start;
  for (int i = 1; i < rows.size(); ++i) {
    if (rows[i] == end + 1) {
      // 连续行，扩展范围
      end = rows[i];
    } else {
      // 不连续，保存当前范围并开始新范围
      ranges.push_back(std::make_pair(start, end));
      start = rows[i];
      end = start;
    }
  }
  // 添加最后一个范围
  ranges.append(std::make_pair(start, end));
  return ranges;
}

}  // namespace ModelTools
