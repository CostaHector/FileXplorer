#ifndef SELECTIONSRANGEHELPER_H
#define SELECTIONSRANGEHELPER_H

#include <QModelIndex>
#include <QSet>

struct SelectionsRangeHelper {
public:
  bool contain(const QString& rootpath, const QModelIndex& index) const {
    return currentPath == rootpath && indexesSet.contains(index);
  }

  void clear() {
    currentPath.clear();
    topLeft2BottomLeftLst.clear();
    indexesSet.clear();
  }

  void Set(const QString& rootpath, const QModelIndexList& rhs) {
    currentPath = rootpath;
    if (rhs.isEmpty()) {return;}
    int begin = 0;
    int end = 0;
    for (int i = 1; i < rhs.size(); ++i) {
      const QModelIndex& current = rhs[i];
      if (current.row() == rhs[end].row() + 1) {
        end = i;
      } else {
        topLeft2BottomLeftLst.append({rhs[begin], rhs[end]});
        begin = i;
        end = i;
      }
    }
    topLeft2BottomLeftLst.append({rhs[begin], rhs[end]});

    QSet<QModelIndex> indexesAppend {rhs.cbegin(), rhs.cend()};
    indexesSet.swap(indexesAppend);
  }

  QList<std::pair<QModelIndex, QModelIndex>> GetTopBottomRange() const {
    return topLeft2BottomLeftLst;
  }

private:
  QString currentPath;
  QList<std::pair<QModelIndex, QModelIndex>> topLeft2BottomLeftLst;
  QSet<QModelIndex> indexesSet;
};

#endif // SELECTIONSRANGEHELPER_H
