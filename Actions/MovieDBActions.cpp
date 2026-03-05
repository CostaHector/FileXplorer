#include "MovieDBActions.h"
#include "PublicMacro.h"

MovieDBActions& g_dbAct() {
  static MovieDBActions ins;
  return ins;
}

MovieDBActions::MovieDBActions(QObject* parent)  //
    : QObject{parent}                            //
{
  SUBMIT = new (std::nothrow) QAction(QIcon(":img/MANUAL_SUBMIT"), tr("Submit"), this);
  CHECK_NULLPTR_RETURN_VOID(SUBMIT);
  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setToolTip(QString("<b>%1 (%2)</b><br/> Manual Submit.").arg(SUBMIT->text(), SUBMIT->shortcut().toString()));

  _MODEL_REPOPULATE = new (std::nothrow) QAction{QIcon(":img/REFRESH_THIS_PATH"), tr("Repopulate"), this};
  CHECK_NULLPTR_RETURN_VOID(_MODEL_REPOPULATE);
  _MODEL_REPOPULATE->setShortcutVisibleInContextMenu(true);
  _MODEL_REPOPULATE->setToolTip(QString{"<b>%1 (%2)</b><br/>Repopulates the model with data from the table via setTable"}.arg(
      _MODEL_REPOPULATE->text(), _MODEL_REPOPULATE->shortcut().toString()));

  REVERT = new (std::nothrow) QAction(QIcon(":img/REVERT"), tr("Revert"), this);
  CHECK_NULLPTR_RETURN_VOID(REVERT);

  INIT_A_DATABASE = new (std::nothrow) QAction(QIcon(":img/CREATE_DATABASE"), tr("Init Database"), this);
  CHECK_NULLPTR_RETURN_VOID(INIT_A_DATABASE);
  INIT_A_DATABASE->setToolTip("CREATE DATABASE `DB_NAME`;");

  INIT_A_TABLE = new (std::nothrow) QAction(QIcon(":img/CREATE_TABLE"), tr("Create Table"), this);
  CHECK_NULLPTR_RETURN_VOID(INIT_A_TABLE);
  INIT_A_TABLE->setToolTip("CREATE TABLE IF NOT EXISTS `DB_TABLE::MOVIES`(`COLUMN_NAME` TEXT NOT NULL, PRIMARY KEY (COLUMN_NAME));");

  DROP_A_TABLE = new (std::nothrow) QAction(QIcon(":img/DROP_TABLE"), tr("Drop Table"), this);
  CHECK_NULLPTR_RETURN_VOID(DROP_A_TABLE);
  DROP_A_TABLE->setToolTip("DROP TABLE `DB_TABLE::MOVIES`;");

  INSERT_A_PATH = new (std::nothrow) QAction(QIcon(":img/INSERT_INTO_TABLE"), tr("Insert Into"), this);
  CHECK_NULLPTR_RETURN_VOID(INSERT_A_PATH);
  INSERT_A_PATH->setToolTip("INSERT INTO `DB_TABLE::MOVIES`(COLUMN_NAME) VALUES (VALUE);");

  DELETE_FROM_TABLE = new (std::nothrow) QAction(QIcon(":img/DELETE_FROM_TABLE"), tr("Delete Where"), this);
  CHECK_NULLPTR_RETURN_VOID(DELETE_FROM_TABLE);
  DELETE_FROM_TABLE->setToolTip("DELETE FROM `DB_TABLE::MOVIES` WHERE CLAUSE");

  UNION_TABLE = new (std::nothrow) QAction(QIcon(":img/UNION"), tr("Union Into"), this);
  CHECK_NULLPTR_RETURN_VOID(UNION_TABLE);
  UNION_TABLE->setToolTip(
      "Merge data from T1 and T2 into MOVIES table: "
      "Conflicting rows (by PRIMARY/UNIQUE key) will be replaced, "
      "non-conflicting existing rows in MOVIES are preserved. "
      "\nSQL: REPLACE INTO `DB_TABLE::MOVIES` SELECT * FROM `T1` UNION SELECT * FROM `T2`;"
      );

  AUDIT_A_TABLE = new (std::nothrow) QAction(QIcon(":img/AUDIT"), tr("Audit This Table"), this);
  CHECK_NULLPTR_RETURN_VOID(AUDIT_A_TABLE);
  AUDIT_A_TABLE->setToolTip("Audit a table (Principle: Compare video records in table with an online disk);");

  DB_CONTROL_ACTIONS = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(DB_CONTROL_ACTIONS);
  DB_CONTROL_ACTIONS->addAction(SUBMIT);
  DB_CONTROL_ACTIONS->addAction(_MODEL_REPOPULATE);
  DB_CONTROL_ACTIONS->addAction(REVERT);
  DB_CONTROL_ACTIONS->addAction(INIT_A_TABLE);
  DB_CONTROL_ACTIONS->addAction(INSERT_A_PATH);
  DB_CONTROL_ACTIONS->addAction(DELETE_FROM_TABLE);
  DB_CONTROL_ACTIONS->addAction(INIT_A_DATABASE);
  DB_CONTROL_ACTIONS->addAction(DROP_A_TABLE);
  DB_CONTROL_ACTIONS->addAction(UNION_TABLE);
  DB_CONTROL_ACTIONS->addAction(AUDIT_A_TABLE);
  DB_CONTROL_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  READ_DURATION_BY_VIDEO = new (std::nothrow) QAction(QIcon(":img/VIDEO_DURATION"), tr("Read Duration"), this);
  CHECK_NULLPTR_RETURN_VOID(READ_DURATION_BY_VIDEO);
  EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON = new (std::nothrow) QAction(QIcon(":img/DUMP_INTO_PSON_FILE"), tr("Export to Json"), this);
  CHECK_NULLPTR_RETURN_VOID(EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON);
  EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON->setToolTip("Export Duration/Studio/Cast/Tags to json file, if any field value is valid");
  UPDATE_STUDIO_CAST_TAGS_BY_JSON = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), "Update by Json", this);
  CHECK_NULLPTR_RETURN_VOID(UPDATE_STUDIO_CAST_TAGS_BY_JSON);
  UPDATE_STUDIO_CAST_TAGS_BY_JSON->setToolTip("Update Studio/Cast/Tags fields from json file, only if all field value in json are valid");

  EX_FUNCTION_ACTIONS = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(EX_FUNCTION_ACTIONS);
  EX_FUNCTION_ACTIONS->addAction(READ_DURATION_BY_VIDEO);
  EX_FUNCTION_ACTIONS->addAction(EXPORT_DURATION_STUDIO_CAST_TAGS_TO_JSON);
  EX_FUNCTION_ACTIONS->addAction(UPDATE_STUDIO_CAST_TAGS_BY_JSON);
  EX_FUNCTION_ACTIONS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  _COUNT = new (std::nothrow) QAction(QIcon(":img/COUNTER"), tr("COUNT"), this);
  CHECK_NULLPTR_RETURN_VOID(_COUNT);
  _COUNT->setToolTip("SELECT COUNT(COLUMN) FROM TABLE WHERE 1;");

  _SUM = new (std::nothrow) QAction(QIcon(":img/SUM"), tr("SUM"), this);
  CHECK_NULLPTR_RETURN_VOID(_SUM);
  _SUM->setToolTip("SELECT SUM(COLUMN) FROM TABLE WHERE 1;");

  _OPEN_DB_WITH_LOCAL_APP = new (std::nothrow) QAction{QIcon(":img/SQLITE_APP"), tr("&Open with local app"), this};
  CHECK_NULLPTR_RETURN_VOID(_OPEN_DB_WITH_LOCAL_APP);
  _OPEN_DB_WITH_LOCAL_APP->setToolTip(
      QString("<b>%1 (%2)</b><br/> Open *.db file in local app.<br/>DB Browser(sqlite) and set it open db by default required.")
          .arg(_OPEN_DB_WITH_LOCAL_APP->text(), _OPEN_DB_WITH_LOCAL_APP->shortcut().toString()));

  SET_STUDIO = new (std::nothrow) QAction(QIcon(":img/STUDIOS_LIST_FILE"), tr("Set Studio"), this);

  SET_CAST = new (std::nothrow) QAction(QIcon(":img/CAST_LIST_FILE"), tr("Set Cast"), this);
  APPEND_CAST = new (std::nothrow) QAction(QIcon(":img/CAST_APPEND_INPUT"), tr("Add Cast"), this);
  REMOVE_CAST = new (std::nothrow) QAction(QIcon(":img/CAST_REMOVE"), tr("Rmv Cast"), this);

  SET_TAGS = new (std::nothrow) QAction(QIcon(":img/TAGS_SET"), tr("Set Tags"), this);
  APPEND_TAGS = new (std::nothrow) QAction(QIcon(":img/TAGS_APPEND"), tr("Add Tags"), this);
  REMOVE_TAGS = new (std::nothrow) QAction(QIcon(":img/TAGS_REMOVE"), tr("Rmv Tag"), this);
}
