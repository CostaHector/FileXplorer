#include "CastDBActions.h"
#include <QMenu>
#include <QActionGroup>
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

CastDBActions::CastDBActions(QObject* parent)  //
  : QObject{parent}                          //
{
  SUBMIT = new (std::nothrow) QAction(QIcon(":img/SUBMIT"), "Submit", this);
  CHECK_NULLPTR_RETURN_VOID(SUBMIT);
  INSERT_INTO_TABLE = new (std::nothrow) QAction(QIcon(":img/INSERT_INTO_TABLE"), "Insert into", this);
  CHECK_NULLPTR_RETURN_VOID(INSERT_INTO_TABLE);
  DELETE_RECORDS = new (std::nothrow) QAction(QIcon{":img/DELETE_FROM_TABLE"}, "Delete selected", this);
  CHECK_NULLPTR_RETURN_VOID(DELETE_RECORDS);
  INIT_DATABASE = new (std::nothrow) QAction(QIcon(":img/CREATE_DATABASE"), "Init Database", this);
  CHECK_NULLPTR_RETURN_VOID(INIT_DATABASE);
  INIT_TABLE = new (std::nothrow) QAction(QIcon(":img/CREATE_TABLE"), "Create table", this);
  CHECK_NULLPTR_RETURN_VOID(INIT_TABLE);
  DELETE_TABLE = new (std::nothrow) QAction(QIcon{":img/DELETE_TABLE"}, "Delete table", this);
  CHECK_NULLPTR_RETURN_VOID(DELETE_TABLE);
  DROP_TABLE = new (std::nothrow) QAction(QIcon(":img/DROP_TABLE"), "Drop table", this);
  CHECK_NULLPTR_RETURN_VOID(DROP_TABLE);
  BASIC_TABLE_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(BASIC_TABLE_OP);
  BASIC_TABLE_OP->addAction(SUBMIT);
  BASIC_TABLE_OP->addAction(INSERT_INTO_TABLE);
  BASIC_TABLE_OP->addAction(DELETE_RECORDS);
  BASIC_TABLE_OP->addAction(INIT_DATABASE);
  BASIC_TABLE_OP->addAction(INIT_TABLE);
  BASIC_TABLE_OP->addAction(DELETE_TABLE);
  BASIC_TABLE_OP->addAction(DROP_TABLE);

  SYNC_ALL_RECORDS_VIDS_FROM_DB = new (std::nothrow) QAction{"Sync all vids field", this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_ALL_RECORDS_VIDS_FROM_DB);
  SYNC_ALL_RECORDS_VIDS_FROM_DB->setToolTip("Sync all record(s) Vids field from db");
  SYNC_SELECTED_RECORDS_VIDS_FROM_DB = new (std::nothrow) QAction{QIcon{":img/REFRESH_THIS_PATH"}, "Sync selected vids field", this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_SELECTED_RECORDS_VIDS_FROM_DB);
  SYNC_SELECTED_RECORDS_VIDS_FROM_DB->setToolTip("Sync selected record(s) Vids field from db");
  REFRESH_OP = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(REFRESH_OP);
  REFRESH_OP->addAction(SYNC_ALL_RECORDS_VIDS_FROM_DB);
  REFRESH_OP->addAction(SYNC_SELECTED_RECORDS_VIDS_FROM_DB);

  OPEN_RECORD_IN_FILE_SYSTEM = new (std::nothrow) QAction{QIcon(":img/FOLDER_OF_PICTURES"), "Show in FileSystem", this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_RECORD_IN_FILE_SYSTEM);
  OPEN_RECORD_IN_FILE_SYSTEM->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_O));
  OPEN_RECORD_IN_FILE_SYSTEM->setToolTip(QString("<b>%1 (%2)</b><br/> Reveal selected cast location in system file explorer")
                                             .arg(OPEN_RECORD_IN_FILE_SYSTEM->text(), OPEN_RECORD_IN_FILE_SYSTEM->shortcut().toString()));

  OPEN_DB_WITH_LOCAL_APP = new (std::nothrow) QAction{QIcon(":img/SQLITE_APP"), "Open Database", this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_DB_WITH_LOCAL_APP);
  OPEN_DB_WITH_LOCAL_APP->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O));
  OPEN_DB_WITH_LOCAL_APP->setToolTip(QString("<b>%1 (%2)</b><br/> Open *.db file in local app(DB Browser sqlite). Precondition: It has been set as default app")
                                         .arg(OPEN_DB_WITH_LOCAL_APP->text(), OPEN_DB_WITH_LOCAL_APP->shortcut().toString()));

  FILE_SYSTEM_OP = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(FILE_SYSTEM_OP);
  FILE_SYSTEM_OP->addAction(OPEN_RECORD_IN_FILE_SYSTEM);
  FILE_SYSTEM_OP->addAction(OPEN_DB_WITH_LOCAL_APP);

  APPEND_FROM_PJSON_FILES = new (std::nothrow) QAction{QIcon{":img/RELOAD_FROM_DISK"}, "Append from pjson files", this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_PJSON_FILES);
  APPEND_FROM_PJSON_FILES->setToolTip(QString("<b>%1 (%2)</b><br/>Append casts records from *.pjson files under ${ImageHost}. Override if primary key conflict")
                                          .arg(APPEND_FROM_PJSON_FILES->text(), APPEND_FROM_PJSON_FILES->shortcut().toString()));

  APPEND_FROM_FILE_SYSTEM_STRUCTURE = new (std::nothrow) QAction{QIcon(":img/FOLDER_OPEN"), "Append from file-system structure", this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_FILE_SYSTEM_STRUCTURE);
  APPEND_FROM_FILE_SYSTEM_STRUCTURE->setToolTip("Append casts records from file-system structure under ${ImageHost}");

  APPEND_FROM_MULTILINES_INPUT = new (std::nothrow) QAction{"Append from multi-lines", this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_MULTILINES_INPUT);
  APPEND_FROM_MULTILINES_INPUT->setToolTip("Append casts records from user input multi-lines");
  LOAD_EXTEND_OP = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(LOAD_EXTEND_OP);
  LOAD_EXTEND_OP->addAction(APPEND_FROM_PJSON_FILES);
  LOAD_EXTEND_OP->addAction(APPEND_FROM_FILE_SYSTEM_STRUCTURE);
  LOAD_EXTEND_OP->addAction(APPEND_FROM_MULTILINES_INPUT);

  DUMP_ALL_RECORDS_INTO_PJSON_FILE = new (std::nothrow) QAction{"Dump all records", this};
  CHECK_NULLPTR_RETURN_VOID(DUMP_ALL_RECORDS_INTO_PJSON_FILE);
  DUMP_SELECTED_RECORDS_INTO_PJSON_FILE = new (std::nothrow) QAction(QIcon{":img/EXPORT_TO"}, "Dump selected records", this);
  CHECK_NULLPTR_RETURN_VOID(DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);
  EXPORT_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(EXPORT_OP);
  EXPORT_OP->addAction(DUMP_ALL_RECORDS_INTO_PJSON_FILE);
  EXPORT_OP->addAction(DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);

  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setShortcutVisibleInContextMenu(true);
}

QMenu* CastDBActions::GetRightClickMenu() const {
  auto* m_performerTableMenu = new QMenu(tr("performer table right click menu"));
  m_performerTableMenu->addAction(g_castAct().SYNC_SELECTED_RECORDS_VIDS_FROM_DB);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_castAct().OPEN_RECORD_IN_FILE_SYSTEM);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_castAct().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);
  m_performerTableMenu->setToolTipsVisible(true);
  return m_performerTableMenu;
}

CastDBActions& g_castAct() {
  static CastDBActions CastManagerActIns;
  return CastManagerActIns;
}
