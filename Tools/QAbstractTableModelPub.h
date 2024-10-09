#ifndef QABSTRACTTABLEMODELPUB_H
#define QABSTRACTTABLEMODELPUB_H

#include <QAbstractTableModel>

class QAbstractTableModelPub : public QAbstractTableModel {
 public:
  QAbstractTableModelPub(QObject* parent = nullptr) : QAbstractTableModel{parent} {}
  void RowsCountStartChange(int beforeRow, int afterRow) {
    if (beforeRow == afterRow) {
      return;
    } else if (beforeRow < afterRow) {
      beginInsertRows(QModelIndex(), beforeRow, afterRow - 1);
    } else {
      beginRemoveRows(QModelIndex(), afterRow, beforeRow - 1);
    }
  }
  void RowsCountEndChange(int beforeRow, int afterRow) {
    if (beforeRow == afterRow) {
      if (afterRow > 0 and rowCount() > 0) {
        emit dataChanged(index(0, 0), index(afterRow - 1, columnCount() - 1), {Qt::ItemDataRole::DisplayRole});
      }
    } else if (beforeRow < afterRow) {
      endInsertRows();
    } else {
      endRemoveRows();
    }
  }

  void ColumnsBeginChange(int beforeColumnCnt, int afterColumnCnt){
    if (beforeColumnCnt == afterColumnCnt) {
      return;
    } else if (beforeColumnCnt < afterColumnCnt){
      beginInsertColumns(QModelIndex(), beforeColumnCnt, afterColumnCnt - 1);
    } else {
      beginRemoveColumns(QModelIndex(), afterColumnCnt, beforeColumnCnt - 1);
    }
  }
  void ColumnsEndChange(int beforeColumnCnt, int afterColumnCnt){
    if (beforeColumnCnt == afterColumnCnt) {
      if (afterColumnCnt > 0 and columnCount() > 0) {
        emit dataChanged(index(0, 0), index(rowCount() - 1, afterColumnCnt - 1), {Qt::ItemDataRole::DisplayRole});
      }
    } else if (beforeColumnCnt < afterColumnCnt){
      endInsertColumns();
    }else {
      endRemoveColumns();
    }
  }
};

#endif  // QABSTRACTTABLEMODELPUB_H
