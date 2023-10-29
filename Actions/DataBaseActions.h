#ifndef DATABASEACTIONS_H
#define DATABASEACTIONS_H

#include "PublicVariable.h"
#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

const QString TABLE_NAME = "movies_info";

class DataBaseActions : public QObject {
 public:
  QActionGroup* DRIVE_SEPERATE_SELECTION_AG;
  QActionGroup* DRIVE_BATCH_SELECTION_AG;
  QActionGroup* DB_CONTROL_ACTIONS;
  QAction* DB_VIEW_CLOSE_SHOW;
  QActionGroup* DB_FUNCTIONS;

  explicit DataBaseActions(QObject* parent = nullptr)
      : QObject{parent},
        DRIVE_SEPERATE_SELECTION_AG(Get_DRIVE_SEPERATE_SELECTION_AG()),
        DRIVE_BATCH_SELECTION_AG(Get_DRIVE_BATCH_SELECTION_AG()),
        DB_CONTROL_ACTIONS(Get_DB_CONTROL_ACTIONS()),
        DB_VIEW_CLOSE_SHOW(Get_DB_VIEW_CLOSE_SHOW_Action()),
        DB_FUNCTIONS(Get_DB_FUNCTIONS_Action()){}

  auto Get_DRIVE_SEPERATE_SELECTION_AG() -> QActionGroup* {
    QSqlDatabase con;
    if (QSqlDatabase::connectionNames().contains("custom_connection")) {
      con = QSqlDatabase::database("custom_connection", false);
    } else {
      con = QSqlDatabase::addDatabase("QSQLITE", "custom_connection");
    }
    con.setDatabaseName(SystemPath::FILE_INFO_DATABASE);
    con.open();
    if (not con.isOpen()) {
      qDebug("con cannot open");
      return nullptr;
    }

    QActionGroup* singleDriveSelectAG = new QActionGroup(this);
    QSqlQuery getDrivers(con);
    getDrivers.exec(QString("SELECT DISTINCT Driver from %1;").arg(TABLE_NAME));
    while (getDrivers.next()) {
      const QString& driveFullName = getDrivers.value("Driver").toString();
      QAction* action = new QAction(driveFullName, this);
      singleDriveSelectAG->addAction(action);
    }
    getDrivers.finish();

    singleDriveSelectAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    for (QAction* act : singleDriveSelectAG->actions()) {
      act->setCheckable(true);
      act->setChecked(true);
    }
    return singleDriveSelectAG;
  }
  auto Get_DRIVE_BATCH_SELECTION_AG() -> QActionGroup* {
    QAction* selectAll = new QAction("all", this);
    QAction* deselectAll = new QAction("none", this);
    QAction* selectOnline = new QAction("online", this);

    QActionGroup* onBatchDriveSelect = new QActionGroup(this);
    onBatchDriveSelect->addAction(selectAll);
    onBatchDriveSelect->addAction(deselectAll);
    onBatchDriveSelect->addAction(selectOnline);
    onBatchDriveSelect->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    for (QAction* act : onBatchDriveSelect->actions()) {
      act->setCheckable(false);
    }
    return onBatchDriveSelect;
  }

  auto Get_DB_CONTROL_ACTIONS() -> QActionGroup* {
    QAction* INIT_A_DATABASE = new QAction(QIcon(":/themes/CREATE_DATABASE"), "Create Database", this);
    INIT_A_DATABASE->setToolTip("CREATE DATABASE `DB_NAME`;");

    QAction* INIT_A_TABLE = new QAction(QIcon(":/themes/CREATE_TABLE"), "Create Table", this);
    INIT_A_TABLE->setToolTip("CREATE TABLE IF NOT EXISTS `TABLE_NAME`(`COLUMN_NAME` TEXT NOT NULL, PRIMARY KEY (COLUMN_NAME));");

    QAction* DROP_A_DATABASE = new QAction(QIcon(":/themes/DROP_DATABASE"), "Drop Database", this);
    DROP_A_DATABASE->setToolTip("DROP DATABASE `DB_NAME`;");

    QAction* DROP_A_TABLE = new QAction(QIcon(":/themes/DROP_TABLE"), "Drop Table", this);
    DROP_A_TABLE->setToolTip("DROP TABLE `TABLE_NAME`;");

    QAction* INSERT_A_PATH = new QAction(QIcon(":/themes/INSERT_INTO_TABLE"), "Insert into", this);
    INSERT_A_PATH->setToolTip("INSERT INTO `TABLE_NAME`(COLUMN_NAME) VALUES (VALUE);");

    QAction* DELETE_FROM_TABLE = new QAction(QIcon(":/themes/DELETE_FROM_TABLE"), "Delete from", this);
    DELETE_FROM_TABLE->setToolTip("DELETE FROM `TABLE_NAME` WHERE CLAUSE");

    QActionGroup* databaseControlAG = new QActionGroup(this);
    databaseControlAG->addAction(INSERT_A_PATH);
    databaseControlAG->addAction(DELETE_FROM_TABLE);
    databaseControlAG->addAction(INIT_A_DATABASE);
    databaseControlAG->addAction(INIT_A_TABLE);
    databaseControlAG->addAction(DROP_A_DATABASE);
    databaseControlAG->addAction(DROP_A_TABLE);
    databaseControlAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    for (QAction* act : databaseControlAG->actions()) {
      act->setCheckable(false);
    }
    return databaseControlAG;
  }
  auto Get_DB_VIEW_CLOSE_SHOW_Action() -> QAction* {
    QAction* showDatabase = new QAction(QIcon(":/themes/SHOW_DATABASE"), "show db", this);
    showDatabase->setCheckable(true);
    showDatabase->setChecked(PreferenceSettings().value(MemoryKey::SHOW_DATABASE.name, MemoryKey::SHOW_DATABASE.v).toBool());
    return showDatabase;
  }

  auto Get_DB_FUNCTIONS_Action() -> QActionGroup*{
    QAction* COUNT = new QAction(QIcon(), "COUNT", this);
    COUNT->setToolTip("SELECT COUNT(COLUMN) FROM TABLE WHERE 1;");

    QAction* AVG = new QAction(QIcon(), "AVG", this);
    AVG->setToolTip("SELECT AVG(COLUMN) FROM TABLE WHERE 1;");

    QAction* SUM = new QAction(QIcon(), "SUM", this);
    SUM->setToolTip("SELECT SUM(COLUMN) FROM TABLE WHERE 1;");

    QActionGroup* databaseFunctionsAG = new QActionGroup(this);
    databaseFunctionsAG->addAction(COUNT);
    databaseFunctionsAG->addAction(AVG);
    databaseFunctionsAG->addAction(SUM);
    databaseFunctionsAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    for (QAction* act : databaseFunctionsAG->actions()) {
      act->setCheckable(false);
    }
    return databaseFunctionsAG;
  }


 signals:
};

DataBaseActions& g_dbAct();
#endif  // DATABASEACTIONS_H
