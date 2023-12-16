#ifndef DATABASEACTIONS_H
#define DATABASEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include "PublicVariable.h"

class DataBaseActions : public QObject {
 public:
  QActionGroup* DRIVE_SEPERATE_SELECTION_AG;
  QActionGroup* DRIVE_BATCH_SELECTION_AG;
  QActionGroup* DB_CONTROL_ACTIONS;
  QAction* DB_VIEW_CLOSE_SHOW;
  QActionGroup* DB_FUNCTIONS;
  QActionGroup* DB_RIGHT_CLICK_MENU_AG;
  QAction* QUICK_WHERE_CLAUSE;

  explicit DataBaseActions(QObject* parent = nullptr)
      : QObject{parent},
        DRIVE_SEPERATE_SELECTION_AG(Get_DRIVE_SEPERATE_SELECTION_AG()),
        DRIVE_BATCH_SELECTION_AG(Get_DRIVE_BATCH_SELECTION_AG()),
        DB_CONTROL_ACTIONS(Get_DB_CONTROL_ACTIONS()),
        DB_VIEW_CLOSE_SHOW(Get_DB_VIEW_CLOSE_SHOW_Action()),
        DB_FUNCTIONS(Get_DB_FUNCTIONS_Action()),
        DB_RIGHT_CLICK_MENU_AG(Get_DB_RIGHT_CLICK_MENU_AG()),
        QUICK_WHERE_CLAUSE{new QAction(QIcon(":/themes/QUICK_WHERE_CLAUSE"), "Where clause", this)} {
    QUICK_WHERE_CLAUSE->setToolTip("Construct where clause quickly;");
  }

  auto Get_DRIVE_SEPERATE_SELECTION_AG() -> QActionGroup* {
    QSqlDatabase con;
    if (QSqlDatabase::connectionNames().contains("custom_connection")) {
      con = QSqlDatabase::database("custom_connection", false);
    } else {
      con = QSqlDatabase::addDatabase("QSQLITE", "custom_connection");
    }
    con.setDatabaseName(SystemPath::VIDS_DATABASE);
    con.open();
    if (not con.isOpen()) {
      qDebug("con cannot open");
      return nullptr;
    }

    QActionGroup* singleDriveSelectAG = new QActionGroup(this);
    QSqlQuery getDrivers(con);
    getDrivers.exec(QString("SELECT DISTINCT Driver from %1;").arg(DB_TABLE::MOVIES));
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
    INIT_A_TABLE->setToolTip("CREATE TABLE IF NOT EXISTS `DB_TABLE::MOVIES`(`COLUMN_NAME` TEXT NOT NULL, PRIMARY KEY (COLUMN_NAME));");

    QAction* DROP_A_DATABASE = new QAction(QIcon(":/themes/DROP_DATABASE"), "Drop Database", this);
    DROP_A_DATABASE->setToolTip("DROP DATABASE `DB_NAME`;");

    QAction* DROP_A_TABLE = new QAction(QIcon(":/themes/DROP_TABLE"), "Drop Table", this);
    DROP_A_TABLE->setToolTip("DROP TABLE `DB_TABLE::MOVIES`;");

    QAction* INSERT_A_PATH = new QAction(QIcon(":/themes/INSERT_INTO_TABLE"), "Insert into", this);
    INSERT_A_PATH->setToolTip("INSERT INTO `DB_TABLE::MOVIES`(COLUMN_NAME) VALUES (VALUE);");

    QAction* DELETE_FROM_TABLE = new QAction(QIcon(":/themes/DELETE_FROM_TABLE"), "Delete from", this);
    DELETE_FROM_TABLE->setToolTip("DELETE FROM `DB_TABLE::MOVIES` WHERE CLAUSE");

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
    QAction* showDatabase = new QAction(QIcon(":/themes/SHOW_DATABASE"), "Movies", this);
    showDatabase->setCheckable(true);
    showDatabase->setChecked(PreferenceSettings().value(MemoryKey::SHOW_DATABASE.name, MemoryKey::SHOW_DATABASE.v).toBool());
    return showDatabase;
  }

  auto Get_DB_FUNCTIONS_Action() -> QActionGroup* {
    QAction* COUNT = new QAction(QIcon(), "COUNT", this);
    COUNT->setToolTip("SELECT COUNT(COLUMN) FROM TABLE WHERE 1;");

    QAction* SUM = new QAction(QIcon(), "SUM", this);
    SUM->setToolTip("SELECT SUM(COLUMN) FROM TABLE WHERE 1;");

    QActionGroup* databaseFunctionsAG = new QActionGroup(this);
    databaseFunctionsAG->addAction(COUNT);
    databaseFunctionsAG->addAction(SUM);
    databaseFunctionsAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    for (QAction* act : databaseFunctionsAG->actions()) {
      act->setCheckable(false);
    }
    return databaseFunctionsAG;
  }

  auto Get_DB_RIGHT_CLICK_MENU_AG() -> QActionGroup* {
    QAction* OPEN_RUN = new QAction("&Open");
    OPEN_RUN->setShortcutVisibleInContextMenu(true);

    QAction* PLAY_VIDEOS = new QAction(QIcon(":/themes/PLAY_BUTTON_TRIANGLE"), "Play Videos");
    PLAY_VIDEOS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
    PLAY_VIDEOS->setShortcutVisibleInContextMenu(true);
    PLAY_VIDEOS->setToolTip(QString("<b>%1 (%2)</b><br/> Play videos").arg(PLAY_VIDEOS->text(), PLAY_VIDEOS->shortcut().toString()));

    QAction* _REVEAL_IN_EXPLORER = new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), "Reveal in Explorer");
    _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
    _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
    _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%0 (%1)</b><br/> Reveal items in system file explorer.")
                                        .arg(_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));
    _REVEAL_IN_EXPLORER->setCheckable(false);

    QAction* DELETE_BY_DRIVER = new QAction(QIcon(), "Delete by driver");
    QAction* DELETE_BY_PREPATH = new QAction(QIcon(), "Delete by prepath");

    QActionGroup* dbRightClickMenuAG = new QActionGroup(this);
    dbRightClickMenuAG->addAction(OPEN_RUN);
    dbRightClickMenuAG->addAction(PLAY_VIDEOS);
    dbRightClickMenuAG->addAction(_REVEAL_IN_EXPLORER);
    dbRightClickMenuAG->addAction(DELETE_BY_DRIVER);
    dbRightClickMenuAG->addAction(DELETE_BY_PREPATH);
    return dbRightClickMenuAG;
  }

 signals:
};

DataBaseActions& g_dbAct();
#endif  // DATABASEACTIONS_H
