#ifndef QABSTRACTLISTMODELPUB_H
#define QABSTRACTLISTMODELPUB_H
#include <QAbstractListModel>

class QAbstractListModelPub : public QAbstractListModel {
 public:
  QAbstractListModelPub(QObject* parent = nullptr) : QAbstractListModel{parent} {}
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
      qWarning("row count[bef:%d, aft:%d] invalid", m_befRow, m_aftRow);
      return;
    }
    if (m_befRow == m_aftRow) {
      if (m_aftRow > 0) {
        emit dataChanged(index(0), index(m_aftRow - 1),  //
                         {Qt::ItemDataRole::DisplayRole | Qt::ItemDataRole::DecorationRole});
      }
    } else if (m_befRow < m_aftRow) {
      endInsertRows();
    } else {
      endRemoveRows();
    }
  }

 private:
  bool IsRowCntValid() const { return m_befRow >= 0 || m_aftRow >= 0; }
  int m_befRow{-1}, m_aftRow{-1};
};
#endif  // QABSTRACTLISTMODELPUB_H
