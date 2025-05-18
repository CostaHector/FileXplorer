#include "JsonProxyModel.h"

JsonProxyModel::JsonProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {}

bool JsonProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  const QModelIndex& index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
  return sourceModel()->data(index).toString().contains(filterRegularExpression());
}
