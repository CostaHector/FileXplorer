#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include "Component/DatabaseSearchToolBar.h"
#include "Component/QuickWhereClause.h"
#include "MyQSqlTableModel.h"
#include "View/CustomTableView.h"

#include <QComboBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLayout>
#include <QMessageBox>
#include <QTableView>

class DatabaseTableView : public CustomTableView {
 public:
  DatabaseTableView(DatabaseSearchToolBar* dbSearchBar, MyQSqlTableModel* dbModel, QWidget* parent = nullptr);

  void subscribe();
  auto on_cellDoubleClicked(QModelIndex clickedIndex) -> bool;
  auto on_PlayVideo() const -> bool;

  auto keyPressEvent(QKeyEvent* e) -> void override {
    if (e->key() == Qt::Key_Return or e->key() == Qt::Key_Enter) {
      on_cellDoubleClicked(currentIndex());
      return;
    }
    QTableView::keyPressEvent(e);
  }

  bool onSearchDataBase(const QString& searchText) {
    _dbModel->setFilter(searchText);
    return true;
  }

  bool InitMoviesTables();
  bool setCurrentMovieTable(const QString& movieTableName);

  bool onUnionTables();

  bool onInitDataBase();
  void onCreateATable();
  bool onDropATable();
  bool onDeleteFromTable(const QString& clause = "");

  bool on_DeleteByDrive();
  bool on_DeleteByPrepath();

  bool onInsertIntoTable();

  void onQuickWhereClause();

  int onCountRow();

  MyQSqlTableModel* _dbModel;

 private:
  DatabaseSearchToolBar* _dbSearchBar;
  QComboBox* _tables;
  QLineEdit* _searchLE;
  QComboBox* _searchCB;

  QuickWhereClause* m_quickWhereClause;
};

#endif  // DATABASETABLEVIEW_H
