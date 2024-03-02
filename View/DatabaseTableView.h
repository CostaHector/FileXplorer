#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include "Component/DBRightClickMenu.h"
#include "Component/QuickWhereClause.h"
#include "Component/DatabaseSearchToolBar.h"
#include "MyQSqlTableModel.h"

#include <QComboBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTableView>
#include <QLayout>


class DatabaseTableView : public QTableView {
 public:
  DatabaseTableView(DatabaseSearchToolBar* dbSearchBar, MyQSqlTableModel* dbModel, QWidget* parent=nullptr);

  auto InitViewSettings() -> void;
  auto UpdateItemViewFontSize() -> void;
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

  void on_ShowContextMenu(const QPoint pnt) {
    m_vidsDBMenu->popup(this->mapToGlobal(pnt));  // or QCursor::pos()
  }

  bool onSearchDataBase(const QString& searchText) {
    _dbModel->setFilter(searchText);
    return true;
  }

  bool ShowOrHideColumnCore();
  bool onHideThisColumn();
  bool onShowAllColumn();
  void onStretchLastSection(const bool checked);


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

  void onSelectBatch(const QAction* act);


  void onQuickWhereClause();

  int onCountRow();


  MyQSqlTableModel* _dbModel;

 private:
  DBRightClickMenu* m_vidsDBMenu;

  QAction* SHOW_ALL_COLUMNS;
  QAction* HIDE_THIS_COLUMN;
  QAction* STRETCH_DETAIL_SECTION;
  QMenu* m_horizontalHeaderMenu;
  int m_horizontalHeaderSectionClicked = -1;
  QString m_columnsShowSwitch;

  DatabaseSearchToolBar* _dbSearchBar;
  QComboBox* _tables;
  QLineEdit* _searchLE;
  QComboBox* _searchCB;

  QuickWhereClause* m_quickWhereClause;
};

#endif  // DATABASETABLEVIEW_H
