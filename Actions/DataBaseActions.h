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

  QActionGroup* Get_DB_CONTROL_ACTIONS();

  QAction* INIT_A_DATABASE = new (std::nothrow) QAction(QIcon(":img/CREATE_DATABASE"), tr("Create Database"), this);
  QAction* INIT_A_TABLE = new (std::nothrow) QAction(QIcon(":img/CREATE_TABLE"), tr("Create Table"), this);
  QAction* DROP_A_TABLE = new (std::nothrow) QAction(QIcon(":img/DROP_TABLE"), tr("Drop Table"), this);
  QAction* INSERT_A_PATH = new (std::nothrow) QAction(QIcon(":img/INSERT_INTO_TABLE"), tr("Insert into"), this);
  QAction* DELETE_FROM_TABLE = new (std::nothrow) QAction(QIcon(":img/DELETE_FROM_TABLE"), tr("Delete from"), this);
  QAction* UNION_TABLE = new (std::nothrow) QAction(QIcon(":img/UNION"), tr("Union"), this);

  QAction* QUICK_WHERE_CLAUSE{new (std::nothrow) QAction(QIcon(":img/QUICK_WHERE_CLAUSE"), tr("Where clause"), this)};

  QAction *_COUNT{nullptr}, *_SUM{nullptr};

  QActionGroup* DB_CONTROL_ACTIONS;
};

DataBaseActions& g_dbAct();
#endif  // DATABASEACTIONS_H
