#include "CastDBActions.h"
#include <QMenu>
#include <QActionGroup>
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"

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

  REFRESH_ALL_RECORDS_VIDS = new (std::nothrow) QAction("Refresh all", this);
  CHECK_NULLPTR_RETURN_VOID(REFRESH_ALL_RECORDS_VIDS);
  REFRESH_SELECTED_RECORDS_VIDS = new (std::nothrow) QAction(QIcon{":img/REFRESH_THIS_PATH"}, "Refresh selected", this);
  CHECK_NULLPTR_RETURN_VOID(REFRESH_SELECTED_RECORDS_VIDS);
  REFRESH_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(REFRESH_OP);
  REFRESH_OP->addAction(REFRESH_ALL_RECORDS_VIDS);
  REFRESH_OP->addAction(REFRESH_SELECTED_RECORDS_VIDS);

  OPEN_RECORD_IN_FILE_SYSTEM = new (std::nothrow) QAction(QIcon(":img/FOLDER_OF_PICTURES"), "Show in FileSystem", this);
  CHECK_NULLPTR_RETURN_VOID(OPEN_RECORD_IN_FILE_SYSTEM);
  OPEN_DB_WITH_LOCAL_APP = new (std::nothrow) QAction(QIcon(":img/SQLITE_APP"), "Open Database", this);
  CHECK_NULLPTR_RETURN_VOID(OPEN_DB_WITH_LOCAL_APP);
  LOCATE_IMAGEHOST = new (std::nothrow) QAction(QIcon(":img/PIN"), "Locate imagehost", this);
  CHECK_NULLPTR_RETURN_VOID(LOCATE_IMAGEHOST);
  FILE_SYSTEM_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(FILE_SYSTEM_OP);
  FILE_SYSTEM_OP->addAction(OPEN_RECORD_IN_FILE_SYSTEM);
  FILE_SYSTEM_OP->addAction(OPEN_DB_WITH_LOCAL_APP);
  FILE_SYSTEM_OP->addAction(LOCATE_IMAGEHOST);

  LOAD_FROM_PJSON_PATH = new (std::nothrow) QAction(QIcon{":img/RELOAD_FROM_DISK"}, "From pjson", this);
  CHECK_NULLPTR_RETURN_VOID(LOAD_FROM_PJSON_PATH);
  LOAD_FROM_FILE_SYSTEM_STRUCTURE = new (std::nothrow) QAction(QIcon(":img/FOLDER_OPEN"), "From file-system structure", this);
  CHECK_NULLPTR_RETURN_VOID(LOAD_FROM_FILE_SYSTEM_STRUCTURE);
  LOAD_FROM_PERFORMERS_LIST = new (std::nothrow) QAction("From Cast lines", this);
  CHECK_NULLPTR_RETURN_VOID(LOAD_FROM_PERFORMERS_LIST);
  LOAD_EXTEND_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(LOAD_EXTEND_OP);
  LOAD_EXTEND_OP->addAction(LOAD_FROM_PJSON_PATH);
  LOAD_EXTEND_OP->addAction(LOAD_FROM_FILE_SYSTEM_STRUCTURE);
  LOAD_EXTEND_OP->addAction(LOAD_FROM_PERFORMERS_LIST);

  DUMP_ALL_RECORDS_INTO_PJSON_FILE = new (std::nothrow) QAction("Dump all", this);
  CHECK_NULLPTR_RETURN_VOID(DUMP_ALL_RECORDS_INTO_PJSON_FILE);
  DUMP_SELECTED_RECORDS_INTO_PJSON_FILE = new (std::nothrow) QAction(QIcon{":img/EXPORT_TO"}, "Dump selected", this);
  CHECK_NULLPTR_RETURN_VOID(DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);
  EXPORT_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(EXPORT_OP);
  EXPORT_OP->addAction(DUMP_ALL_RECORDS_INTO_PJSON_FILE);
  EXPORT_OP->addAction(DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);

  CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT = new (std::nothrow) QAction("Cast image height(px)", this);
  CHECK_NULLPTR_RETURN_VOID(CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT);

  HORIZONTAL_HEADER_AGS = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(HORIZONTAL_HEADER_AGS);
  HORIZONTAL_HEADER_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  VERTICAL_HEADER_AGS = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(VERTICAL_HEADER_AGS);
//  VERTICAL_HEADER_AGS->addAction(DELETE_RECORDS);

  LOAD_FROM_PJSON_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> Load *.pjson from ImageHost.<br/>Update each column value by new one if primary key conflicts.")
                                       .arg(LOAD_FROM_PJSON_PATH->text(), LOAD_FROM_PJSON_PATH->shortcut().toString()));

  OPEN_DB_WITH_LOCAL_APP->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O));
  OPEN_DB_WITH_LOCAL_APP->setToolTip(QString("<b>%1 (%2)</b><br/> Open *.db file in local app.<br/>DB Browser(sqlite) and set it open db by default required.")
                                         .arg(OPEN_DB_WITH_LOCAL_APP->text(), OPEN_DB_WITH_LOCAL_APP->shortcut().toString()));
  LOCATE_IMAGEHOST->setToolTip(PreferenceSettings().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v).toString());

  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setShortcutVisibleInContextMenu(true);

  const QString& heightStr = QString::number(PreferenceSettings().value(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.v).toInt());
  CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT->setToolTip(heightStr);
}

QMenuBar* CastDBActions::GetMenuBar() const {
  auto* fileMenu = new QMenu(tr("File"));
  fileMenu->addActions({LOAD_FROM_PJSON_PATH, LOAD_FROM_FILE_SYSTEM_STRUCTURE, LOAD_FROM_PERFORMERS_LIST});
  fileMenu->addSeparator();
  fileMenu->addActions({DUMP_ALL_RECORDS_INTO_PJSON_FILE, DUMP_SELECTED_RECORDS_INTO_PJSON_FILE});
  fileMenu->addSeparator();
  fileMenu->addActions({REFRESH_ALL_RECORDS_VIDS, REFRESH_SELECTED_RECORDS_VIDS});
  fileMenu->addSeparator();
  fileMenu->addAction(OPEN_DB_WITH_LOCAL_APP);
  fileMenu->addSeparator();
  fileMenu->addAction(LOCATE_IMAGEHOST);
  fileMenu->setToolTipsVisible(true);

  auto* editMenu = new QMenu(tr("Edit"));
  editMenu->addActions({INIT_DATABASE, INIT_TABLE, INSERT_INTO_TABLE});
  editMenu->addSeparator();
  editMenu->addActions({DELETE_TABLE, DROP_TABLE});
  editMenu->addSeparator();
  editMenu->addAction(SUBMIT);
  editMenu->setToolTipsVisible(true);

  auto* viewMenu = new QMenu(tr("View"));
  viewMenu->addActions({CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT});
  viewMenu->setToolTipsVisible(true);

  auto* m_menuBar = new QMenuBar;
  m_menuBar->addMenu(fileMenu);
  m_menuBar->addMenu(editMenu);
  m_menuBar->addMenu(viewMenu);
  return m_menuBar;
}

QMenu* CastDBActions::GetRightClickMenu() const {
  auto* m_performerTableMenu = new QMenu(tr("performer table right click menu"));
  m_performerTableMenu->addAction(g_castAct().REFRESH_SELECTED_RECORDS_VIDS);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_castAct().OPEN_RECORD_IN_FILE_SYSTEM);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_castAct().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);
  m_performerTableMenu->setToolTipsVisible(true);
  return m_performerTableMenu;
}

QActionGroup* CastDBActions::GetVerAGS() const {
  return g_castAct().VERTICAL_HEADER_AGS;
}
QActionGroup* CastDBActions::GetHorAGS() const {
  return g_castAct().HORIZONTAL_HEADER_AGS;
}

CastDBActions& g_castAct() {
  static CastDBActions performersManagerActIns;
  return performersManagerActIns;
}
