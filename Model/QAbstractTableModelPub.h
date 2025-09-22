#ifndef QABSTRACTTABLEMODELPUB_H
#define QABSTRACTTABLEMODELPUB_H

#include <QAbstractTableModel>
#include "Logger.h"

class QAbstractTableModelPub : public QAbstractTableModel {
 public:
  QAbstractTableModelPub(QObject* parent = nullptr) : QAbstractTableModel{parent} {}

  void RowsCountBeginChange(int beforeRow, int afterRow) {
    m_befRow = beforeRow;
    m_aftRow = afterRow;
    if (m_befRow == m_aftRow) {
      return;
    } else if (m_befRow < m_aftRow) {
      beginInsertRows(QModelIndex(), m_befRow, m_aftRow - 1);
    } else {
      beginRemoveRows(QModelIndex(), m_aftRow, m_befRow - 1);
    }
  }
  void RowsCountEndChange() {
    if (!IsRowCntValid()) {
      LOG_W("row count[bef:%d, aft:%d] invalid", m_befRow, m_aftRow);
      return;
    }
    if (m_befRow == m_aftRow) {
      if (m_aftRow > 0 && columnCount() > 0) {
        emit dataChanged(index(0, 0), index(m_aftRow - 1, columnCount() - 1), {Qt::ItemDataRole::DisplayRole | Qt::ItemDataRole::DecorationRole});
      }
    } else if (m_befRow < m_aftRow) {
      endInsertRows();
    } else {
      endRemoveRows();
    }
  }

  template<typename SwappableContainerDataType>
  void RowsCountChange(SwappableContainerDataType& lhs, SwappableContainerDataType& rhs);

  void ColumnsCountBeginChange(int beforeColumnCnt, int afterColumnCnt) {
    m_befCol = beforeColumnCnt;
    m_aftCol = afterColumnCnt;
    if (m_befCol == m_aftCol) {
      return;
    } else if (m_befCol < m_aftCol) {
      beginInsertColumns(QModelIndex(), m_befCol, m_aftCol - 1);
    } else {
      beginRemoveColumns(QModelIndex(), m_aftCol, m_befCol - 1);
    }
  }
  void ColumnsCountEndChange() {
    if (!IsColCntValid()) {
      LOG_W("col count[bef:%d, aft:%d] invalid", m_befCol, m_aftCol);
      return;
    }
    if (m_befCol == m_aftCol) {
      if (m_aftCol > 0 && rowCount() > 0) {
        emit dataChanged(index(0, 0), index(rowCount() - 1, m_aftCol - 1), {Qt::ItemDataRole::DisplayRole | Qt::ItemDataRole::DecorationRole});
      }
    } else if (m_befCol < m_aftCol) {
      endInsertColumns();
    } else {
      endRemoveColumns();
    }
  }

 private:
  bool IsRowCntValid() const { return m_befRow >= 0 || m_aftRow >= 0; }
  bool IsColCntValid() const { return m_befCol >= 0 || m_aftCol >= 0; }
  int m_befRow{-1}, m_aftRow{-1};
  int m_befCol{-1}, m_aftCol{-1};
};

extern template void QAbstractTableModelPub::RowsCountChange<QStringList>(QStringList&, QStringList&);

#endif  // QABSTRACTTABLEMODELPUB_H
