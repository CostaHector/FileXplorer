#include "DataBaseActions.h"

DataBaseActions& g_dbAct() {
  static DataBaseActions ins;
  return ins;
}

DataBaseActions::DataBaseActions(QObject* parent)    //
    : QObject{parent},                               //
      DB_CONTROL_ACTIONS(Get_DB_CONTROL_ACTIONS()) {
  QUICK_WHERE_CLAUSE->setToolTip("Construct where clause quickly;");
  _COUNT = new (std::nothrow) QAction(QIcon(":img/COUNTER"), tr("COUNT"), this);
  _COUNT->setToolTip("SELECT COUNT(COLUMN) FROM TABLE WHERE 1;");

  _SUM = new (std::nothrow) QAction(QIcon(":img/SUM"), tr("SUM"), this);
  _SUM->setToolTip("SELECT SUM(COLUMN) FROM TABLE WHERE 1;");
}

QActionGroup* DataBaseActions::Get_DB_CONTROL_ACTIONS() {
  INIT_A_DATABASE->setToolTip("CREATE DATABASE `DB_NAME`;");

  INIT_A_TABLE->setToolTip("CREATE TABLE IF NOT EXISTS `DB_TABLE::MOVIES`(`COLUMN_NAME` TEXT NOT NULL, PRIMARY KEY (COLUMN_NAME));");

  DROP_A_TABLE->setToolTip("DROP TABLE `DB_TABLE::MOVIES`;");

  INSERT_A_PATH->setToolTip("INSERT INTO `DB_TABLE::MOVIES`(COLUMN_NAME) VALUES (VALUE);");

  DELETE_FROM_TABLE->setToolTip("DELETE FROM `DB_TABLE::MOVIES` WHERE CLAUSE");

  UNION_TABLE->setToolTip("REPLACE INTO `DB_TABLE::MOVIES` SELECT * FROM `T1` UNION SELECT * FROM `T2`;");

  QActionGroup* databaseControlAG = new (std::nothrow) QActionGroup(this);
  databaseControlAG->addAction(INSERT_A_PATH);
  databaseControlAG->addAction(DELETE_FROM_TABLE);
  databaseControlAG->addAction(INIT_A_DATABASE);
  databaseControlAG->addAction(INIT_A_TABLE);
  databaseControlAG->addAction(DROP_A_TABLE);
  databaseControlAG->addAction(UNION_TABLE);
  databaseControlAG->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  foreach (QAction* act, databaseControlAG->actions()) {
    act->setCheckable(false);
  }
  return databaseControlAG;
}
