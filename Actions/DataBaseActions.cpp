#include "DataBaseActions.h"
#include "public/PublicMacro.h"

DataBaseActions& g_dbAct() {
  static DataBaseActions ins;
  return ins;
}

DataBaseActions::DataBaseActions(QObject* parent)  //
    : QObject{parent}                              //
{
  SUBMIT = new (std::nothrow) QAction(QIcon(":img/SUBMIT"), "Submit", this);
  CHECK_NULLPTR_RETURN_VOID(SUBMIT);
  INIT_A_DATABASE = new (std::nothrow) QAction(QIcon(":img/CREATE_DATABASE"), "Create Database", this);
  CHECK_NULLPTR_RETURN_VOID(INIT_A_DATABASE);
  INIT_A_TABLE = new (std::nothrow) QAction(QIcon(":img/CREATE_TABLE"), "Create Table", this);
  CHECK_NULLPTR_RETURN_VOID(INIT_A_TABLE);
  DROP_A_TABLE = new (std::nothrow) QAction(QIcon(":img/DROP_TABLE"), "Drop Table", this);
  CHECK_NULLPTR_RETURN_VOID(DROP_A_TABLE);
  INSERT_A_PATH = new (std::nothrow) QAction(QIcon(":img/INSERT_INTO_TABLE"), "Insert Into", this);
  CHECK_NULLPTR_RETURN_VOID(INSERT_A_PATH);
  DELETE_FROM_TABLE = new (std::nothrow) QAction(QIcon(":img/DELETE_FROM_TABLE"), "Delete From", this);
  CHECK_NULLPTR_RETURN_VOID(DELETE_FROM_TABLE);
  UNION_TABLE = new (std::nothrow) QAction(QIcon(":img/UNION"), "Union Into", this);
  CHECK_NULLPTR_RETURN_VOID(UNION_TABLE);
  AUDIT_A_TABLE = new (std::nothrow) QAction(QIcon(":img/AUDIT"), "Audit This Table", this);
  CHECK_NULLPTR_RETURN_VOID(AUDIT_A_TABLE);

  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setToolTip(QString("<b>%1 (%2)</b><br/> Manual Submit.").arg(SUBMIT->text(), SUBMIT->shortcut().toString()));
  INIT_A_DATABASE->setToolTip("CREATE DATABASE `DB_NAME`;");
  INIT_A_TABLE->setToolTip("CREATE TABLE IF NOT EXISTS `DB_TABLE::MOVIES`(`COLUMN_NAME` TEXT NOT NULL, PRIMARY KEY (COLUMN_NAME));");
  DROP_A_TABLE->setToolTip("DROP TABLE `DB_TABLE::MOVIES`;");
  INSERT_A_PATH->setToolTip("INSERT INTO `DB_TABLE::MOVIES`(COLUMN_NAME) VALUES (VALUE);");
  DELETE_FROM_TABLE->setToolTip("DELETE FROM `DB_TABLE::MOVIES` WHERE CLAUSE");
  UNION_TABLE->setToolTip("REPLACE INTO `DB_TABLE::MOVIES` SELECT * FROM `T1` UNION SELECT * FROM `T2`;");
  AUDIT_A_TABLE->setToolTip("Audit a table (Principle: Compare video records in table with an online disk);");

  DB_CONTROL_ACTIONS = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(DB_CONTROL_ACTIONS);
  DB_CONTROL_ACTIONS->addAction(SUBMIT);
  DB_CONTROL_ACTIONS->addAction(INSERT_A_PATH);
  DB_CONTROL_ACTIONS->addAction(DELETE_FROM_TABLE);
  DB_CONTROL_ACTIONS->addAction(INIT_A_DATABASE);
  DB_CONTROL_ACTIONS->addAction(INIT_A_TABLE);
  DB_CONTROL_ACTIONS->addAction(DROP_A_TABLE);
  DB_CONTROL_ACTIONS->addAction(UNION_TABLE);
  DB_CONTROL_ACTIONS->addAction(AUDIT_A_TABLE);
  DB_CONTROL_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  SET_DURATION_BY_VIDEO = new (std::nothrow) QAction(QIcon(":img/VIDEO_DURATION"), "Set Duration", this);
  CHECK_NULLPTR_RETURN_VOID(SET_DURATION_BY_VIDEO);
  EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON = new (std::nothrow) QAction(QIcon(":img/EXPORT_TO"), "Export to Json", this);
  CHECK_NULLPTR_RETURN_VOID(EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON);
  UPDATE_STUDIO_CAST_TAGS_BY_JSON = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), "Update by Json", this);
  CHECK_NULLPTR_RETURN_VOID(UPDATE_STUDIO_CAST_TAGS_BY_JSON);

  EX_FUNCTION_ACTIONS = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(EX_FUNCTION_ACTIONS);
  EX_FUNCTION_ACTIONS->addAction(SET_DURATION_BY_VIDEO);
  EX_FUNCTION_ACTIONS->addAction(EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON);
  EX_FUNCTION_ACTIONS->addAction(UPDATE_STUDIO_CAST_TAGS_BY_JSON);
  EX_FUNCTION_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  QUICK_WHERE_CLAUSE = new (std::nothrow) QAction(QIcon(":img/QUICK_WHERE_CLAUSE"), tr("Where clause"), this);
  CHECK_NULLPTR_RETURN_VOID(QUICK_WHERE_CLAUSE);
  QUICK_WHERE_CLAUSE->setToolTip("Construct where clause quickly;");

  _COUNT = new (std::nothrow) QAction(QIcon(":img/COUNTER"), tr("COUNT"), this);
  CHECK_NULLPTR_RETURN_VOID(_COUNT);
  _COUNT->setToolTip("SELECT COUNT(COLUMN) FROM TABLE WHERE 1;");

  _SUM = new (std::nothrow) QAction(QIcon(":img/SUM"), tr("SUM"), this);
  CHECK_NULLPTR_RETURN_VOID(_SUM);
  _SUM->setToolTip("SELECT SUM(COLUMN) FROM TABLE WHERE 1;");
}
