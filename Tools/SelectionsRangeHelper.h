#ifndef SELECTIONSRANGEHELPER_H
#define SELECTIONSRANGEHELPER_H

#include <QModelIndex>
#include <QItemSelection>
#include <QSet>

struct SelectionsRangeHelper {
public:
  bool contain(const QString& rootpath, const QModelIndex& index) const {
    return currentPath == rootpath && indexesSet.contains(index);
  }

  void clear() {
    currentPath.clear();
    selections.clear();
    indexesSet.clear();
  }

  void Set(const QString& rootpath, const QItemSelection& rhs) {
    currentPath = rootpath;
    selections = rhs;
    const auto& indexesList = rhs.indexes();
    QSet<QModelIndex> indexesAppend {indexesList.cbegin(), indexesList.cend()};
    indexesSet.swap(indexesAppend);
  }

  QList<std::pair<QModelIndex, QModelIndex>> GetTopBottomRange() const {
    if (selections.isEmpty()) {
      return {};
    }
    decltype(GetTopBottomRange()) ans;
    ans.reserve(selections.size());
    const auto& mdl = selections.front().model();
    const auto& par = selections.front().parent();
    for (const auto& rng: selections) {
      ans.push_back(std::make_pair(rng.topLeft(), mdl->index(rng.bottom(), rng.left(), par)));
    }
    return ans;
  }

private:
  QString currentPath;
  QItemSelection selections;
  QSet<QModelIndex> indexesSet;
};

#endif // SELECTIONSRANGEHELPER_H
