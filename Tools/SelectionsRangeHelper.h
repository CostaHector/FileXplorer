#ifndef SELECTIONSRANGEHELPER_H
#define SELECTIONSRANGEHELPER_H

#include <QModelIndex>
#include <bitset>

class SelectionsRangeHelper {
public:
  bool contains(const QString& rootpath, unsigned row) const {
    return currentPath == rootpath && row < MAX_INDEX_CNT && mSelectedRowBits.test(row);
  }

  void clear() {
    currentPath.clear();
    mRowRangeList.clear();
    mSelectedRowBits.reset();
  }

  void Set(const QString& rootpath, const QModelIndexList& selectedRows) {
    clear();
    currentPath = rootpath;

    if (selectedRows.isEmpty()) {return;}
    for (const QModelIndex& ind: selectedRows) {
      mSelectedRowBits.set(ind.row());
    }
    if (selectedRows.size() == selectedRows.back().row() - selectedRows.front().row() + 1) { // [topRow, bottomRow]
      mRowRangeList.append({selectedRows.front().row(), selectedRows.back().row()});
      return;
    }

    int top = 0, bottom = 0; // [top, bottom]
    for (int i = 1; i < selectedRows.size(); ++i) {
      if (selectedRows[i].row() != selectedRows[bottom].row() + 1) {
        mRowRangeList.append({selectedRows[top].row(), selectedRows[bottom].row()});
        top = i;
      }
      bottom = i;
    }
    mRowRangeList.append({selectedRows[top].row(), selectedRows[bottom].row()});
  }

  QList<std::pair<int, int>> GetTopBottomRange() const {
    return mRowRangeList;
  }

private:
  QString currentPath;
  QList<std::pair<int, int>> mRowRangeList; // [front, back] contain endpoint
  static constexpr int MAX_INDEX_CNT = 4096;
  std::bitset<MAX_INDEX_CNT> mSelectedRowBits{0};
};
#endif // SELECTIONSRANGEHELPER_H
