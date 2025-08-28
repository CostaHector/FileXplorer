#include "CastDBActions.h"
#include <QMenu>
#include <QActionGroup>
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

CastDBActions::CastDBActions(QObject* parent)  //
  : QObject{parent}                          //
{
  SUBMIT = new (std::nothrow) QAction{QIcon(":img/SUBMIT"), "Submit", this};
  CHECK_NULLPTR_RETURN_VOID(SUBMIT);
  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setShortcutVisibleInContextMenu(true);

  APPEND_FROM_MULTILINES_INPUT = new (std::nothrow) QAction{QIcon(":img/INSERT_INTO_TABLE"), "Append Casts", this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_MULTILINES_INPUT);
  APPEND_FROM_MULTILINES_INPUT->setToolTip("Append casts records from user input multi-lines");
  APPEND_FROM_PSON_FILES = new (std::nothrow) QAction{"Append from pson files", this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_PSON_FILES);
  APPEND_FROM_PSON_FILES->setToolTip(QString("<b>%1 (%2)</b><br/>Append casts records from *.pson files under ${ImageHost}. Override if primary key conflict")
    .arg(APPEND_FROM_PSON_FILES->text(), APPEND_FROM_PSON_FILES->shortcut().toString()));
  APPEND_FROM_FILE_SYSTEM_STRUCTURE = new (std::nothrow) QAction{"Append from file-system structure", this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_FILE_SYSTEM_STRUCTURE);
  APPEND_FROM_FILE_SYSTEM_STRUCTURE->setToolTip("Append casts records from file-system structure under ${ImageHost}");

  DELETE_RECORDS = new (std::nothrow) QAction{QIcon{":img/DELETE_FROM_TABLE"}, "Delete selected", this};
  CHECK_NULLPTR_RETURN_VOID(DELETE_RECORDS);
  INIT_DATABASE = new (std::nothrow) QAction{QIcon{":img/CREATE_DATABASE"}, "Init Database", this};
  CHECK_NULLPTR_RETURN_VOID(INIT_DATABASE);
  INIT_TABLE = new (std::nothrow) QAction{QIcon{":img/CREATE_TABLE"}, "Create table", this};
  CHECK_NULLPTR_RETURN_VOID(INIT_TABLE);
  DELETE_TABLE = new (std::nothrow) QAction{QIcon{":img/DELETE_TABLE"}, "Delete table", this};
  CHECK_NULLPTR_RETURN_VOID(DELETE_TABLE);
  DROP_TABLE = new (std::nothrow) QAction{QIcon{":img/DROP_TABLE"}, "Drop table", this};
  CHECK_NULLPTR_RETURN_VOID(DROP_TABLE);
  BASIC_TABLE_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(BASIC_TABLE_OP);
  BASIC_TABLE_OP->addAction(DELETE_RECORDS);
  BASIC_TABLE_OP->addAction(INIT_DATABASE);
  BASIC_TABLE_OP->addAction(INIT_TABLE);
  BASIC_TABLE_OP->addAction(DELETE_TABLE);
  BASIC_TABLE_OP->addAction(DROP_TABLE);

  SYNC_SELECTED_RECORDS_IMGS_FROM_DISK = new (std::nothrow) QAction{QIcon{":img/SYNC_FROM_DISK"}, "Sync imgs field", this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_SELECTED_RECORDS_IMGS_FROM_DISK);
  SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->setShortcut(QKeySequence(Qt::Key::Key_F5));
  SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->setToolTip(QString{"<b>%1 (%2)</b><br/> Sync selected record(s) Imgs field from disk"}//
    .arg(SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->text(), SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->shortcut().toString()));
  SYNC_ALL_RECORDS_IMGS_FROM_DISK = new (std::nothrow) QAction{"Sync all imgs field", this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_ALL_RECORDS_IMGS_FROM_DISK);
  SYNC_ALL_RECORDS_IMGS_FROM_DISK->setToolTip("Sync all record(s) Imgs field from disk");
  _SYNC_IMGS_OP = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(_SYNC_IMGS_OP);
  _SYNC_IMGS_OP->addAction(SYNC_SELECTED_RECORDS_IMGS_FROM_DISK);
  _SYNC_IMGS_OP->addAction(SYNC_ALL_RECORDS_IMGS_FROM_DISK);

  SYNC_SELECTED_RECORDS_VIDS_FROM_DB = new (std::nothrow) QAction{QIcon{":img/SYNC_FROM_DB"}, "Sync vids field", this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_SELECTED_RECORDS_VIDS_FROM_DB);
  SYNC_SELECTED_RECORDS_VIDS_FROM_DB->setToolTip("Sync selected record(s) Vids field from db");
  SYNC_ALL_RECORDS_VIDS_FROM_DB = new (std::nothrow) QAction{"Sync all vids field", this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_ALL_RECORDS_VIDS_FROM_DB);
  SYNC_ALL_RECORDS_VIDS_FROM_DB->setToolTip("Sync all record(s) Vids field from db");
  _SYNC_VIDS_OP = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(_SYNC_VIDS_OP);
  _SYNC_VIDS_OP->addAction(SYNC_SELECTED_RECORDS_VIDS_FROM_DB);
  _SYNC_VIDS_OP->addAction(SYNC_ALL_RECORDS_VIDS_FROM_DB);

  MIGRATE_CAST_TO = new (std::nothrow) QAction{QIcon(":img/MIGRATE_CAST_TO"), "Migrate to", this};
  CHECK_NULLPTR_RETURN_VOID(MIGRATE_CAST_TO);
  MIGRATE_CAST_TO->setToolTip(QString("<b>%1 (%2)</b><br/> Migrate selected cast path to user specified one")
                                             .arg(MIGRATE_CAST_TO->text(), MIGRATE_CAST_TO->shortcut().toString()));
  OPEN_DB_WITH_LOCAL_APP = new (std::nothrow) QAction{QIcon(":img/SQLITE_APP"), "Open Database", this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_DB_WITH_LOCAL_APP);
  OPEN_DB_WITH_LOCAL_APP->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O));
  OPEN_DB_WITH_LOCAL_APP->setToolTip(QString("<b>%1 (%2)</b><br/> Open *.db file in local app(DB Browser sqlite). Precondition: It has been set as default app")
    .arg(OPEN_DB_WITH_LOCAL_APP->text(), OPEN_DB_WITH_LOCAL_APP->shortcut().toString()));
  FILE_SYSTEM_OP = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(FILE_SYSTEM_OP);
  FILE_SYSTEM_OP->addAction(MIGRATE_CAST_TO);
  FILE_SYSTEM_OP->addAction(OPEN_DB_WITH_LOCAL_APP);

  DUMP_SELECTED_RECORDS_INTO_PSON_FILE = new (std::nothrow) QAction{QIcon{":img/DUMP_INTO_PSON_FILE"}, "Dump records", this};
  CHECK_NULLPTR_RETURN_VOID(DUMP_SELECTED_RECORDS_INTO_PSON_FILE);
  DUMP_SELECTED_RECORDS_INTO_PSON_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key::Key_S));
  DUMP_SELECTED_RECORDS_INTO_PSON_FILE->setToolTip(QString{"<b>%1 (%2)</b><br/> Dump selected records to its pson file"}//
    .arg(DUMP_SELECTED_RECORDS_INTO_PSON_FILE->text(), DUMP_SELECTED_RECORDS_INTO_PSON_FILE->shortcut().toString()));
  DUMP_ALL_RECORDS_INTO_PSON_FILE = new (std::nothrow) QAction{"Dump all records", this};
  CHECK_NULLPTR_RETURN_VOID(DUMP_ALL_RECORDS_INTO_PSON_FILE);
  DUMP_ALL_RECORDS_INTO_PSON_FILE->setToolTip("Dump all records to its pson file");
  EXPORT_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(EXPORT_OP);
  EXPORT_OP->addAction(DUMP_SELECTED_RECORDS_INTO_PSON_FILE);
  EXPORT_OP->addAction(DUMP_ALL_RECORDS_INTO_PSON_FILE);
}

QToolButton* CastDBActions::GetAppendCastToolButton(QWidget* parent) {
  auto* menu = new (std::nothrow) QMenu{parent};
  menu->addAction(APPEND_FROM_PSON_FILES);
  menu->addAction(APPEND_FROM_FILE_SYSTEM_STRUCTURE);

  auto* tb = new (std::nothrow) QToolButton{parent};
  tb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  tb->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
  tb->setDefaultAction(APPEND_FROM_MULTILINES_INPUT);
  tb->setMenu(menu);
  return tb;
}

QMenu* CastDBActions::GetRightClickMenu(QWidget* parent) const {
  auto* castTableMenu = new (std::nothrow) QMenu{"Cast book RightClickMenu", parent};
  castTableMenu->addAction(g_castAct().SYNC_SELECTED_RECORDS_VIDS_FROM_DB);
  castTableMenu->addAction(g_castAct().SYNC_SELECTED_RECORDS_IMGS_FROM_DISK);
  castTableMenu->addAction(g_castAct().DUMP_SELECTED_RECORDS_INTO_PSON_FILE);
  castTableMenu->addSeparator();
  castTableMenu->addAction(MIGRATE_CAST_TO);
  castTableMenu->setToolTipsVisible(true);
  return castTableMenu;
}

CastDBActions& g_castAct() {
  static CastDBActions CastManagerActIns;
  return CastManagerActIns;
}
