#include "DataBaseActions.h"

DataBaseActions& g_dbAct() {
  static DataBaseActions ins;
  return ins;
}

DataBaseActions::DataBaseActions(QObject* parent)
    : QObject{parent},
      DB_CONTROL_ACTIONS(Get_DB_CONTROL_ACTIONS()),
      DB_FUNCTIONS(Get_DB_FUNCTIONS_Action()),
      DB_RIGHT_CLICK_MENU_AG(Get_DB_RIGHT_CLICK_MENU_AG()) {
  QUICK_WHERE_CLAUSE->setToolTip("Construct where clause quickly;");
}

QActionGroup* DataBaseActions::Get_DB_CONTROL_ACTIONS() {
  INIT_A_DATABASE->setToolTip("CREATE DATABASE `DB_NAME`;");

  INIT_A_TABLE->setToolTip("CREATE TABLE IF NOT EXISTS `DB_TABLE::MOVIES`(`COLUMN_NAME` TEXT NOT NULL, PRIMARY KEY (COLUMN_NAME));");

  DROP_A_DATABASE->setToolTip("DROP DATABASE `DB_NAME`;");

  DROP_A_TABLE->setToolTip("DROP TABLE `DB_TABLE::MOVIES`;");

  INSERT_A_PATH->setToolTip("INSERT INTO `DB_TABLE::MOVIES`(COLUMN_NAME) VALUES (VALUE);");

  DELETE_FROM_TABLE->setToolTip("DELETE FROM `DB_TABLE::MOVIES` WHERE CLAUSE");

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

  foreach (QAction* act, databaseControlAG->actions()) {
    act->setCheckable(false);
  }
  return databaseControlAG;
}

QActionGroup* DataBaseActions::Get_DB_FUNCTIONS_Action() {
  QAction* COUNT = new QAction(QIcon(":img/COUNTER"), tr("COUNT"), this);
  COUNT->setToolTip("SELECT COUNT(COLUMN) FROM TABLE WHERE 1;");

  QAction* SUM = new QAction(QIcon(":img/SUM"), tr("SUM"), this);
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

QActionGroup* DataBaseActions::Get_DB_RIGHT_CLICK_MENU_AG() {
  QActionGroup* dbRightClickMenuAG = new QActionGroup(this);
  dbRightClickMenuAG->addAction(DELETE_BY_DRIVER);
  dbRightClickMenuAG->addAction(DELETE_BY_PREPATH);
  return dbRightClickMenuAG;
}
