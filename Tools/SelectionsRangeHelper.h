#ifndef SELECTIONSRANGEHELPER_H
#define SELECTIONSRANGEHELPER_H

#include <QModelIndex>
#include <QSet>

struct SelectionsRangeHelper {
public:
  bool contains(const QString& rootpath, int row) const {
    return currentPath == rootpath && indexesSet.contains(row);
  }

  void clear() {
    currentPath.clear();
    topLeft2BottomLeftLst.clear();
    indexesSet.clear();
  }

  void Set(const QString& rootpath, const QModelIndexList& rhs) {
    clear();
    currentPath = rootpath;

    if (rhs.isEmpty()) {return;}

    for (const QModelIndex& ind: rhs) {
      indexesSet.insert(ind.row());
    }

    if (rhs.size() == rhs.back().row() - rhs.front().row() + 1) { // [topRow, bottomRow]
      topLeft2BottomLeftLst.append({rhs.front().row(), rhs.back().row()});
      return;
    }

    int top = 0;
    int bottom = 0; // [top, bottom]
    for (int i = 1; i < rhs.size(); ++i) {
      const QModelIndex& current = rhs[i];
      if (current.row() == rhs[bottom].row() + 1) {
        bottom = i;
      } else {
        topLeft2BottomLeftLst.append({rhs[top].row(), rhs[bottom].row()});
        top = i;
        bottom = i;
      }
    }
    topLeft2BottomLeftLst.append({rhs[top].row(), rhs[bottom].row()});
  }

  QList<std::pair<int, int>> GetTopBottomRange() const {
    return topLeft2BottomLeftLst;
  }

private:
  QString currentPath;
  QList<std::pair<int, int>> topLeft2BottomLeftLst; // [front, back] contain endpoint
  QSet<int> indexesSet;
};

#endif // SELECTIONSRANGEHELPER_H
