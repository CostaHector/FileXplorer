#ifndef SELECTIONSRANGEHELPER_H
#define SELECTIONSRANGEHELPER_H

#include <QModelIndex>
#include <bitset>

class SelectionsRangeHelper {
public:
  typedef QList<std::pair<QModelIndex, QModelIndex>> ROW_RANGES_LST;

  QString GetCurrentPath() const {return currentPath;}

  bool contains(const QString& rootpath, unsigned row) const {
    return currentPath == rootpath && row < MAX_INDEX_CNT && mSelectedRowBits.test(row);
  }

  void clear() {
    currentPath.clear();
    mRowRangeList.clear();
    mSelectedRowBits.reset();
  }

  bool isEmpty() const {return mRowRangeList.isEmpty(); }

  void Set(const QString& rootpath, const QModelIndexList& selectedRows) {
    clear();
    currentPath = rootpath;

    if (selectedRows.isEmpty()) {return;}
    for (const QModelIndex& ind: selectedRows) {
      mSelectedRowBits.set(ind.row());
    }
    if (selectedRows.size() == selectedRows.back().row() - selectedRows.front().row() + 1) { // [topRow, bottomRow]
      mRowRangeList.append({selectedRows.front(), selectedRows.back()});
      return;
    }

    int top = 0, bottom = 0; // [top, bottom]
    for (int i = 1; i < selectedRows.size(); ++i) {
      if (selectedRows[i].row() != selectedRows[bottom].row() + 1) {
        mRowRangeList.append({selectedRows[top], selectedRows[bottom]});
        top = i;
      }
      bottom = i;
    }
    mRowRangeList.append({selectedRows[top], selectedRows[bottom]});
  }

  const ROW_RANGES_LST& GetTopBottomRange() const {
    return mRowRangeList;
  }
private:
  QString currentPath;
  ROW_RANGES_LST mRowRangeList; // [front, back] contain endpoint
  static constexpr int MAX_INDEX_CNT = 4096;
  std::bitset<MAX_INDEX_CNT> mSelectedRowBits{0};
};
#endif // SELECTIONSRANGEHELPER_H
