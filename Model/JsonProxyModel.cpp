#include "JsonProxyModel.h"

bool JsonProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  const QString keyword = filterRegularExpression().pattern(); // Qt 6
  if (keyword.isEmpty()) {
    return true;
  }
  const QModelIndex& index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
  return sourceModel()->data(index).toString().contains(keyword);
}
