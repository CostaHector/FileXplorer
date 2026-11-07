#include "ViewSelectionHelper.h"
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include "PublicCheckMacro.h"

namespace ViewSelectionHelper {
QModelIndexList selectedRowsSourceCore(const QAbstractItemView& view) {
  const QItemSelectionModel* pSelectionModel = view.selectionModel();
  CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(pSelectionModel);
  const QModelIndexList selectedIndexes{pSelectionModel->selectedRows()};
  // model() can be proxy
  QAbstractProxyModel* _proxyModel = qobject_cast<QAbstractProxyModel*>(view.model());
  if (_proxyModel == nullptr) {
    return selectedIndexes;
  }
  QModelIndexList srcIndexes;
  srcIndexes.reserve(selectedIndexes.size());
  for (const QModelIndex& proxyIndex : selectedIndexes) {
    srcIndexes.push_back(_proxyModel->mapToSource(proxyIndex));
  }
  return srcIndexes;
}
}