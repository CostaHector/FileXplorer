#ifndef DATABASEACTIONS_H
#define DATABASEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QObject>

class DataBaseActions : public QObject {
  Q_OBJECT
 public:
  explicit DataBaseActions(QObject* parent = nullptr);

  auto Get_DB_CONTROL_ACTIONS() -> QActionGroup*;

  auto Get_DB_FUNCTIONS_Action() -> QActionGroup*;

  auto Get_DB_RIGHT_CLICK_MENU_AG() -> QActionGroup*;

  QAction* INIT_A_DATABASE = new QAction(QIcon(":/themes/CREATE_DATABASE"), tr("Create Database"), this);
  QAction* INIT_A_TABLE = new QAction(QIcon(":/themes/CREATE_TABLE"), tr("Create Table"), this);
  QAction* DROP_A_DATABASE = new QAction(QIcon(":/themes/DROP_DATABASE"), tr("Drop Database"), this);
  QAction* DROP_A_TABLE = new QAction(QIcon(":/themes/DROP_TABLE"), tr("Drop Table"), this);
  QAction* INSERT_A_PATH = new QAction(QIcon(":/themes/INSERT_INTO_TABLE"), tr("Insert into"), this);
  QAction* DELETE_FROM_TABLE = new QAction(QIcon(":/themes/DELETE_FROM_TABLE"), tr("Delete from"), this);
  QAction* UNION_TABLE = new QAction(QIcon(":/themes/UNION"), tr("Union"), this);

  QAction* DELETE_BY_DRIVER = new QAction(QIcon(), tr("Delete by driver"));
  QAction* DELETE_BY_PREPATH = new QAction(QIcon(), tr("Delete by prepath"));

  QAction* QUICK_WHERE_CLAUSE{new QAction(QIcon(":/themes/QUICK_WHERE_CLAUSE"), tr("Where clause"), this)};

  QActionGroup* DB_CONTROL_ACTIONS;
  QActionGroup* DB_FUNCTIONS;
  QActionGroup* DB_RIGHT_CLICK_MENU_AG;
};

DataBaseActions& g_dbAct();
#endif  // DATABASEACTIONS_H
