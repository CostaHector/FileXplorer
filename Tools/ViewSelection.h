#ifndef VIEWSELECTION_H
#define VIEWSELECTION_H

#include <QModelIndexList>
#include <QAbstractItemView>

namespace ViewSelection {
QModelIndexList selectedIndexes(const QAbstractItemView* view);
}

#endif // VIEWSELECTION_H
