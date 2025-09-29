#include "QAbstractTableModelPub.h"
#include "Logger.h"

bool QAbstractTableModelPub::RowsCountBeginChange(int beforeRow, int afterRow) {
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

bool QAbstractTableModelPub::RowsCountEndChange() {  // must call me after RowsCountBeginChange called and data change finished
  if (mRowChangeStack.empty()) {
    LOG_W("No row count change at all!");
    return false;
  }
  DimensionRange rng = mRowChangeStack.top();
  mRowChangeStack.pop();
  if (rng.m_bef == rng.m_aft) {
    if (rng.m_aft > 0 && columnCount() > 0) {
      emit dataChanged(index(0, 0), index(rng.m_aft - 1, columnCount() - 1), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole});
    }
  } else if (rng.m_bef < rng.m_aft) {
    endInsertRows();
  } else {
    endRemoveRows();
  }
  return true;
}

bool QAbstractTableModelPub::ColumnsCountBeginChange(int beforeColumnCnt, int afterColumnCnt) {
  if (!IsDimensionCntValid(beforeColumnCnt, afterColumnCnt)) {
    LOG_W("col count[bef:%d, aft:%d] invalid", beforeColumnCnt, afterColumnCnt);
    return false;
  }
  mColumnChangeStack.emplace(beforeColumnCnt, afterColumnCnt);
  if (beforeColumnCnt == afterColumnCnt) {
    return true;
  } else if (beforeColumnCnt < afterColumnCnt) {
    beginInsertColumns(QModelIndex(), beforeColumnCnt, afterColumnCnt - 1);
  } else {
    beginRemoveColumns(QModelIndex(), afterColumnCnt, beforeColumnCnt - 1);
  }
  return true;
}
bool QAbstractTableModelPub::ColumnsCountEndChange() {  // must call me after ColumnsCountBeginChange called and data change finished
  if (mColumnChangeStack.empty()) {
    LOG_W("No column count change at all!");
    return false;
  }
  DimensionRange rng = mColumnChangeStack.top();
  mColumnChangeStack.pop();
  if (rng.m_bef == rng.m_aft) {
    if (rng.m_aft > 0 && rowCount() > 0) {
      emit dataChanged(index(0, 0), index(rowCount() - 1, rng.m_aft - 1), {Qt::ItemDataRole::DisplayRole, Qt::ItemDataRole::DecorationRole});
    }
  } else if (rng.m_bef < rng.m_aft) {
    endInsertColumns();
  } else {
    endRemoveColumns();
  }
  return true;
}

template <typename Swappable2DimContainerDataType>
void QAbstractTableModelPub::DimensionCountChange(Swappable2DimContainerDataType& lhs,
                                                  Swappable2DimContainerDataType& rhs,
                                                  const DataChangeRangeE changeRangeE) {
  if (changeRangeE == DataChangeRangeE::BOTH_ROW_AND_COLUMN) {
    beginResetModel();
    lhs.swap(rhs);
    endResetModel();
    return;
  }
  if ((int)changeRangeE & (int)DataChangeRangeE::ROW) {
    const int beforeRowCnt = lhs.size(), afterRowCnt = rhs.size();
    RowsCountBeginChange(beforeRowCnt, afterRowCnt);
    lhs.swap(rhs);
    RowsCountEndChange();
    return;
  }
  if ((int)changeRangeE & (int)DataChangeRangeE::COLUMN) {
    int beforeColumnCnt = 0;
    if (!lhs.isEmpty()) {
      beforeColumnCnt = lhs[0].size();  // user should assure Swappable2DimContainerDataType second dimension is all same one value
    }
    int afterColumnCnt = 0;
    if (!rhs.isEmpty()) {
      afterColumnCnt = rhs[0].size();
    }
    ColumnsCountBeginChange(beforeColumnCnt, afterColumnCnt);
    lhs.swap(rhs);
    ColumnsCountEndChange();
    return;
  }
  LOG_E("Unknow changeRangeE[%d]", (int)changeRangeE);
  return;
}

template void QAbstractTableModelPub::DimensionCountChange<QList<QStringList>>(QList<QStringList>& lhs,
                                                                               QList<QStringList>& rhs,
                                                                               const QAbstractTableModelPub::DataChangeRangeE changeRangeE);
