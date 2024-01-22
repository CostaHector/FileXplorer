#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include "Component/DBRightClickMenu.h"
#include "Component/QuickWhereClause.h"
#include "MyQSqlTableModel.h"

#include <QComboBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTableView>

class DatabaseTableView : public QTableView {
 public:
  DatabaseTableView();

  auto InitViewSettings() -> void;
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
    m_vidsDBMenu->popup(this->mapToGlobal(pnt));  // or QCursor::pos()
  }

  bool onSearchDataBase(const QString& searchText) {
    m_dbModel->setFilter(searchText);
    return true;
  }

  bool ShowOrHideColumnCore();
  bool onHideThisColumn();
  bool onShowAllColumn();
  void onStretchLastSection(const bool checked);

  MyQSqlTableModel* m_dbModel;

 private:
  DBRightClickMenu* m_vidsDBMenu;

  QAction* SHOW_ALL_COLUMNS;
  QAction* HIDE_THIS_COLUMN;
  QAction* STRETCH_DETAIL_SECTION;
  QMenu* m_horizontalHeaderMenu;
  int m_horizontalHeaderSectionClicked = -1;
  QString m_columnsShowSwitch;
};

class DatabasePanel : public QWidget {
 public:
  explicit DatabasePanel(QWidget* parent = nullptr);

  bool InitMoviesTables();
  bool setCurrentMovieTable(const QString& movieTableName);

  bool onSearchDataBase(const QString& searchText);

  bool onUnionTables();

  bool onInitDataBase();
  void onCreateATable();
  bool onDropATable();
  bool onDeleteFromTable(const QString& clause = "");

  bool on_DeleteByDrive();
  bool on_DeleteByPrepath();

  bool onInsertIntoTable();

  void onSelectBatch(const QAction* act);

  void subscribe();

  void onQuickWhereClause();


  int onCountRow();

  QComboBox* m_tables;
  QLineEdit* m_searchLE;
  QComboBox* m_searchCB;
  DatabaseTableView* m_dbView;

  QuickWhereClause* m_quickWhereClause;
};

#endif  // DATABASETABLEVIEW_H
