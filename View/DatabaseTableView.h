#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include <QTableView>
#include "MyQSqlTableModel.h"
class DatabaseTableView : public QTableView {
 public:
  MyQSqlTableModel* dbModel;

  DatabaseTableView();

  auto InitViewSettings() -> void;
  auto SetViewColumnWidth() -> void;
  auto UpdateItemViewFontSize() -> void;
  void subscribe();
  auto on_cellDoubleClicked(QModelIndex clickedIndex) -> bool;
};

#endif  // DATABASETABLEVIEW_H
