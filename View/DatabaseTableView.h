#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include "Component/DatabaseSearchToolBar.h"
#include "Component/MovieDatabaseMenu.h"
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
  auto on_PlayVideo() const -> bool;

  auto keyPressEvent(QKeyEvent* e) -> void override { QTableView::keyPressEvent(e); }

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

  QString getMovieTableName() const { return m_movieTableName; }

 private:
  MyQSqlTableModel* _dbModel;
  MovieDatabaseMenu* m_movieMenu;
  DatabaseSearchToolBar* _dbSearchBar;
  QComboBox* _tables;
  QLineEdit* _searchLE;
  QComboBox* _searchCB;

  QuickWhereClause* m_quickWhereClause;

  QString m_movieTableName;
};

#endif  // DATABASETABLEVIEW_H
