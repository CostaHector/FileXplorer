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
  Q_OBJECT
 public:
  QActionGroup* DB_CONTROL_ACTIONS;
  QActionGroup* DB_FUNCTIONS;

  QAction* OPEN_RUN = nullptr;
  QAction* _PLAY_VIDEOS = nullptr;
  QAction* _REVEAL_IN_EXPLORER = nullptr;
  QAction* COPY_DB_ITEM_NAME = nullptr;
  QAction* COPY_DB_ITEM_FULL_PATH = nullptr;
  QAction* DELETE_BY_DRIVER = nullptr;
  QAction* DELETE_BY_PREPATH = nullptr;
  QActionGroup* DB_RIGHT_CLICK_MENU_AG;

  QAction* QUICK_WHERE_CLAUSE;

  explicit DataBaseActions(QObject* parent = nullptr)
      : QObject{parent},
        DB_CONTROL_ACTIONS(Get_DB_CONTROL_ACTIONS()),
        DB_FUNCTIONS(Get_DB_FUNCTIONS_Action()),
        DB_RIGHT_CLICK_MENU_AG(Get_DB_RIGHT_CLICK_MENU_AG()),
        QUICK_WHERE_CLAUSE{new QAction(QIcon(":/themes/QUICK_WHERE_CLAUSE"), tr("Where clause"), this)} {
    QUICK_WHERE_CLAUSE->setToolTip("Construct where clause quickly;");
  }

  auto Get_DB_CONTROL_ACTIONS() -> QActionGroup* {
    QAction* INIT_A_DATABASE = new QAction(QIcon(":/themes/CREATE_DATABASE"), tr("Create Database"), this);
    INIT_A_DATABASE->setToolTip("CREATE DATABASE `DB_NAME`;");

    QAction* INIT_A_TABLE = new QAction(QIcon(":/themes/CREATE_TABLE"), tr("Create Table"), this);
    INIT_A_TABLE->setToolTip("CREATE TABLE IF NOT EXISTS `DB_TABLE::MOVIES`(`COLUMN_NAME` TEXT NOT NULL, PRIMARY KEY (COLUMN_NAME));");

    QAction* DROP_A_DATABASE = new QAction(QIcon(":/themes/DROP_DATABASE"), tr("Drop Database"), this);
    DROP_A_DATABASE->setToolTip("DROP DATABASE `DB_NAME`;");

    QAction* DROP_A_TABLE = new QAction(QIcon(":/themes/DROP_TABLE"), tr("Drop Table"), this);
    DROP_A_TABLE->setToolTip("DROP TABLE `DB_TABLE::MOVIES`;");

    QAction* INSERT_A_PATH = new QAction(QIcon(":/themes/INSERT_INTO_TABLE"), tr("Insert into"), this);
    INSERT_A_PATH->setToolTip("INSERT INTO `DB_TABLE::MOVIES`(COLUMN_NAME) VALUES (VALUE);");

    QAction* DELETE_FROM_TABLE = new QAction(QIcon(":/themes/DELETE_FROM_TABLE"), tr("Delete from"), this);
    DELETE_FROM_TABLE->setToolTip("DELETE FROM `DB_TABLE::MOVIES` WHERE CLAUSE");

    QAction* UNION_TABLE = new QAction(QIcon(":/themes/UNION"), tr("Union"), this);
    UNION_TABLE->setToolTip("REPLACE INTO `DB_TABLE::MOVIES` SELECT * FROM `T1` UNION SELECT * FROM `T2`;");

    QActionGroup* databaseControlAG = new QActionGroup(this);
    databaseControlAG->addAction(INSERT_A_PATH);
    databaseControlAG->addAction(DELETE_FROM_TABLE);
    databaseControlAG->addAction(INIT_A_DATABASE);
    databaseControlAG->addAction(INIT_A_TABLE);
    databaseControlAG->addAction(DROP_A_DATABASE);
    databaseControlAG->addAction(DROP_A_TABLE);
    databaseControlAG->addAction(UNION_TABLE);
    databaseControlAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

    for (QAction* act : databaseControlAG->actions()) {
      act->setCheckable(false);
    }
    return databaseControlAG;
  }

  auto Get_DB_FUNCTIONS_Action() -> QActionGroup* {
    QAction* COUNT = new QAction(QIcon(":/themes/COUNTER"), tr("COUNT"), this);
    COUNT->setToolTip("SELECT COUNT(COLUMN) FROM TABLE WHERE 1;");

    QAction* SUM = new QAction(QIcon(":/themes/SUM"), tr("SUM"), this);
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
    OPEN_RUN = new QAction(tr("&Open"));
    OPEN_RUN->setShortcutVisibleInContextMenu(true);

    _PLAY_VIDEOS = new QAction(QIcon(":/themes/PLAY_BUTTON_TRIANGLE"), tr("Play Videos"));
    _PLAY_VIDEOS->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Return));
    _PLAY_VIDEOS->setShortcutVisibleInContextMenu(true);
    _PLAY_VIDEOS->setToolTip(QString("<b>%1 (%2)</b><br/> Play videos").arg(_PLAY_VIDEOS->text(), _PLAY_VIDEOS->shortcut().toString()));

    _REVEAL_IN_EXPLORER = new QAction(QIcon(":/themes/REVEAL_IN_EXPLORER"), tr("Reveal in Explorer"));
    _REVEAL_IN_EXPLORER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
    _REVEAL_IN_EXPLORER->setShortcutVisibleInContextMenu(true);
    _REVEAL_IN_EXPLORER->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal items in system file explorer.")
                                        .arg(_REVEAL_IN_EXPLORER->text(), _REVEAL_IN_EXPLORER->shortcut().toString()));
    _REVEAL_IN_EXPLORER->setCheckable(false);

    COPY_DB_ITEM_NAME = new QAction(QIcon(), tr("Copy item name"));
    COPY_DB_ITEM_FULL_PATH = new QAction(QIcon(), tr("Copy item full path to clipboard"));

    DELETE_BY_DRIVER = new QAction(QIcon(), tr("Delete by driver"));
    DELETE_BY_PREPATH = new QAction(QIcon(), tr("Delete by prepath"));

    QActionGroup* dbRightClickMenuAG = new QActionGroup(this);
    dbRightClickMenuAG->addAction(OPEN_RUN);
    dbRightClickMenuAG->addAction(_PLAY_VIDEOS);
    dbRightClickMenuAG->addAction(_REVEAL_IN_EXPLORER);
    dbRightClickMenuAG->addAction(COPY_DB_ITEM_NAME);
    dbRightClickMenuAG->addAction(COPY_DB_ITEM_FULL_PATH);
    dbRightClickMenuAG->addAction(DELETE_BY_DRIVER);
    dbRightClickMenuAG->addAction(DELETE_BY_PREPATH);
    return dbRightClickMenuAG;
  }

 signals:
};

DataBaseActions& g_dbAct();
#endif  // DATABASEACTIONS_H
