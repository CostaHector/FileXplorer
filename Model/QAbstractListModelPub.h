#ifndef QABSTRACTLISTMODELPUB_H
#define QABSTRACTLISTMODELPUB_H
#include <QAbstractListModel>
#include <stack>
#include "Logger.h"
class QAbstractListModelPub : public QAbstractListModel {
 public:
  using QAbstractListModel::QAbstractListModel;
  bool RowsCountBeginChange(int beforeRow, int afterRow) {
    if (!IsDimensionCntValid(beforeRow, afterRow)) {
      LOG_W("row count[bef:%d, aft:%d] invalid", beforeRow, afterRow);
      return false;
    }
    mRowChangeStack.emplace(beforeRow, afterRow);
    if (beforeRow == afterRow) {
      return true;
    } else if (beforeRow < afterRow) {
      beginInsertRows(QModelIndex(), beforeRow, afterRow - 1);
    } else {
      beginRemoveRows(QModelIndex(), afterRow, beforeRow - 1);
    }
    return true;
  }
  bool RowsCountEndChange() {
    if (mRowChangeStack.empty()) {
      LOG_W("No row count change at all!");
      return false;
    }
    DimensionRange rng = mRowChangeStack.top();
    mRowChangeStack.pop();
    if (rng.m_bef == rng.m_aft) {
      if (rng.m_aft > 0) {
        emit dataChanged(index(0, 0), index(rng.m_aft - 1, 0), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole});
      }
    } else if (rng.m_bef < rng.m_aft) {
      endInsertRows();
    } else {
      endRemoveRows();
    }
    return true;
  }

 private:
  struct DimensionRange {
    DimensionRange() = delete;
    DimensionRange(int bef, int aft) : m_bef{bef}, m_aft{aft} {}
    int m_bef;
    int m_aft;
  };
  bool IsDimensionCntValid(int bef, int aft) const { return bef >= 0 && aft >= 0; }
  std::stack<DimensionRange> mRowChangeStack;
};
#endif  // QABSTRACTLISTMODELPUB_H
