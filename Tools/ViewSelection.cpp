#include "ViewSelection.h"

#include <QListView>
#include <QTableView>
#include <QTreeView>
QModelIndexList ViewSelection::selectedIndexes(const QAbstractItemView* view) {
  if (view == nullptr) {
    return {};
  }
  auto* _viewTable = dynamic_cast<const QTableView*>(view);
  if (_viewTable != nullptr) {
    return _viewTable->selectionModel()->selectedRows();
  }
  auto* _viewList = dynamic_cast<const QListView*>(view);
  if (_viewList != nullptr) {
    return _viewList->selectionModel()->selectedRows();
  }
  auto* _viewTree = dynamic_cast<const QTreeView*>(view);
  if (_viewTree != nullptr) {
    return _viewTree->selectionModel()->selectedRows();
  }
  return {};
}
