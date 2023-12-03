#ifndef DATABASETABLEVIEW_H
#define DATABASETABLEVIEW_H

#include "Component/DBRightClickMenu.h"
#include "MyQSqlTableModel.h"
#include "PublicTool.h"

#include <QComboBox>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTableView>

class DatabaseTableView : public QTableView {
 public:
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
    m_vidsDBMenu->popup(this->mapToGlobal(pnt));  // or QCursor::pos()
  }

  auto GetSelectionByDriveClause(const QList<QAction*>& selectByDriveActs) -> QString {
    QStringList clauseLst;
    for (QAction* act : selectByDriveActs) {
      if (act->isChecked()) {
        clauseLst << QString("Driver=\"%1\"").arg(act->text());
      }
    }
    if (clauseLst.size() == selectByDriveActs.size()) {  // select all driver
      return "";
    }
    return clauseLst.join(" OR ");
  }

  bool onSearchDataBase(const QString& searchText) {
    QStringList conditionGroup;
    if (not searchText.isEmpty()) {
      conditionGroup << searchText;
    }

    const QString& driverWhereClause = this->GetSelectionByDriveClause(g_dbAct().DRIVE_SEPERATE_SELECTION_AG->actions());
    if (not driverWhereClause.isEmpty()) {
      conditionGroup << driverWhereClause;
    }

    if (conditionGroup.isEmpty()) {
      m_dbModel->setFilter("");
      return true;
    }

    const QString& whereClause = conditionGroup.join(" AND ");
    m_dbModel->setFilter(whereClause);
    return true;
  }

  int onCountRow() {
    auto con = GetSqlVidsDB();
    if (not con.isOpen()) {
      qDebug("Cannot open connection");
      return -1;
    }
    const QString& countCmd = QString("SELECT COUNT(%1) FROM %2;").arg(DB_HEADER_KEY::Name).arg(TABLE_NAME);

    QSqlQuery queryCount(con);
    queryCount.exec(countCmd);
    queryCount.next();
    const int rowCnt = queryCount.value(0).toInt();

    QMessageBox::information(this, countCmd, QString("%1").arg(rowCnt));
    return rowCnt;
  }

  MyQSqlTableModel* m_dbModel;

 private:
  DBRightClickMenu* m_vidsDBMenu;
};

class DatabasePanel : public QWidget {
 public:
  explicit DatabasePanel(QWidget* parent = nullptr);

  bool onSearchDataBase(const QString& searchText);
  auto onSelectSingleDriver() -> void { m_dbView->onSearchDataBase(m_searchLE->text()); }

  bool onInitDataBase();
  void onInitATable();
  bool onDropATable();
  bool onDeleteFromTable(const QString& clause = "");

  bool on_DeleteByDrive();
  bool on_DeleteByPrepath();

  bool onInsertIntoTable();

  void onSelectBatch(const QAction* act);

  void subscribe();

  void onQuickWhereClause();

  QLineEdit* m_searchLE;
  QComboBox* m_searchCB;
  DatabaseTableView* m_dbView;
};

#endif  // DATABASETABLEVIEW_H
