#ifndef VIEWSELECTION_H
#define VIEWSELECTION_H

#include <QModelIndexList>
#include <QAbstractItemView>

namespace ViewSelection {
auto selectedIndexes(const QAbstractItemView* view) -> QModelIndexList;
};

#endif // VIEWSELECTION_H
