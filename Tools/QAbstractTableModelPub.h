#ifndef QABSTRACTTABLEMODELPUB_H
#define QABSTRACTTABLEMODELPUB_H

#include <QAbstractTableModel>

class QAbstractTableModelPub : public QAbstractTableModel {
 public:
  QAbstractTableModelPub(QObject* parent = nullptr) : QAbstractTableModel{parent} {}
  void RowsCountStartChange(int beforeRow, int afterRow) {
    qDebug("Row changing %d->%d...", beforeRow, afterRow);
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
      qWarning("row count[bef:%d, aft:%d] invalid", m_befRow, m_aftRow);
      return;
    }
    if (m_befRow == m_aftRow) {
      if (m_aftRow > 0 && columnCount() > 0) {
        emit dataChanged(index(0, 0), index(m_aftRow - 1, columnCount() - 1), {Qt::ItemDataRole::DisplayRole});
      }
    } else if (m_befRow < m_aftRow) {
      endInsertRows();
    } else {
      endRemoveRows();
    }
  }

  void ColumnsBeginChange(int beforeColumnCnt, int afterColumnCnt) {
    qDebug("column changing %d->%d...", beforeColumnCnt, afterColumnCnt);
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
  void ColumnsEndChange() {
    if (!IsColCntValid()) {
      qWarning("col count[bef:%d, aft:%d] invalid", m_befCol, m_aftCol);
      return;
    }
    if (m_befCol == m_aftCol) {
      if (m_aftCol > 0 && rowCount() > 0) {
        emit dataChanged(index(0, 0), index(rowCount() - 1, m_aftCol - 1), {Qt::ItemDataRole::DisplayRole});
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

#endif  // QABSTRACTTABLEMODELPUB_H
