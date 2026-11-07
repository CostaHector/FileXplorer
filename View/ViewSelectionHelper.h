#ifndef VIEWSELECTIONHELPER_H
#define VIEWSELECTIONHELPER_H

#include <QModelIndexList>
class QAbstractItemView;

namespace ViewSelectionHelper {
QModelIndexList selectedRowsSourceCore(const QAbstractItemView& view);
}

#endif // VIEWSELECTIONHELPER_H
