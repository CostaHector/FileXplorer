#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include "Component/DBRightClickMenu.h"
#include "MyQSqlTableModel.h"
#include <QTableView>
#include <QKeyEvent>

class DatabaseTableView : public QTableView {
 public:
  MyQSqlTableModel* dbModel;

  DatabaseTableView();

  auto InitViewSettings() -> void;
  auto SetViewColumnWidth() -> void;
  auto UpdateItemViewFontSize() -> void;
  void subscribe();
  auto on_cellDoubleClicked(QModelIndex clickedIndex) -> bool;
  auto on_revealInExplorer() const -> bool;
  auto on_PlayVideo() const -> bool;

  auto keyPressEvent(QKeyEvent* e) -> void override {
    if (e->key() == Qt::Key_Return or e->key() == Qt::Key_Enter) {
      on_cellDoubleClicked(currentIndex());
      return;
    }
    QTableView::keyPressEvent(e);
  }

  void on_ShowContextMenu(const QPoint pnt) {
    menu->popup(this->mapToGlobal(pnt));  // or QCursor::pos()
  }

 private:
    DBRightClickMenu* menu;
};

#endif  // DATABASETABLEVIEW_H
