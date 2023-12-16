#include "PerformersManagerActions.h"
#include <QMenu>
#include "PublicVariable.h"

PerformersManagerActions::PerformersManagerActions(QObject* parent)
    : QObject{parent},
      REFRESH_ALL_RECORDS_VIDS(new QAction("Refresh all record(s) vid", this)),
      REFRESH_SELECTED_RECORDS_VIDS(new QAction("Refresh selected record(s) vid", this)),
      OPEN_RECORD_IN_FILE_SYSTEM(new QAction("Open record", this)),
      LOAD_FROM_PJSON_PATH(new QAction("Load from pjson", this)),
      LOAD_FROM_FILE_SYSTEM_STRUCTURE(new QAction("Load from file-system structure", this)),
      LOAD_FROM_PERFORMERS_LIST(new QAction("Load from performers list", this)),
      DUMP_ALL_RECORDS_INTO_PJSON_FILE(new QAction("Dump all record(s)=>.pjson", this)),
      DUMP_SELECTED_RECORDS_INTO_PJSON_FILE(new QAction("Dump selected record(s)=>.pjson", this)),
      OPEN_WITH_LOCAL_APP(new QAction("Open with", this)),
      LOCATE_IMAGEHOST(new QAction("Locate imagehost", this)),
      INIT_DATABASE(new QAction(QString("Init Database [%1]").arg(SystemPath::PEFORMERS_DATABASE), this)),
      INIT_TABLE(new QAction(QString("Create table [%1]").arg(DB_TABLE::PERFORMERS), this)),
      INSERT_INTO_TABLE(new QAction(QString("insert into table [%1]").arg(DB_TABLE::PERFORMERS), this)),
      DELETE_TABLE(new QAction("Delete table(complete data)", this)),
      DROP_TABLE(new QAction("Drop table(complete table))", this)),
      SUBMIT(new QAction("Submit", this)),

      COLUMNS_VISIBILITY(new QAction("Performer table column visibility", this)),
      CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT(new QAction("Change performer image fixed height(px)", this)),

      HIDE_THIS_COLUMN(new QAction("hide this column", this)),
      SHOW_ALL_COLUMNS(new QAction("show all columns", this)),
      STRETCH_DETAIL_SECTION(new QAction("stretch last column", this)),
      HORIZONTAL_HEADER_AGS(new QActionGroup(this)),

      DELETE_RECORDS(new QAction("delete record(s)", this)),
      RESIZE_ROWS_TO_CONTENT(new QAction("resize rows to content", this)),
      RESIZE_ROWS_DEFAULT_SECTION_SIZE(new QAction("adjust default rows section size", this)),
      VERTICAL_HEADER_AGS(new QActionGroup(this)),

      SHOW_PERFORMER_MANAGER(new QAction(QIcon(":/themes/PERFORMERS_MANAGER"), "Performers Manager", this)),
      m_menuBar(GetMenuBar()) {
  InitActionsTooltips();
}

void PerformersManagerActions::InitActionsTooltips() {
  LOAD_FROM_PJSON_PATH->setToolTip(
      QString("<b>%0 (%1)</b><br/> Load *.pjson from ImageHost.<br/>Update each column value by new one if primary key conflicts.")
          .arg(LOAD_FROM_PJSON_PATH->text(), LOAD_FROM_PJSON_PATH->shortcut().toString()));

  OPEN_WITH_LOCAL_APP->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O));
  OPEN_WITH_LOCAL_APP->setToolTip(
      QString("<b>%0 (%1)</b><br/> Open *.db file in local app.<br/>DB Browser(sqlite) and set it open db by default required.")
          .arg(OPEN_WITH_LOCAL_APP->text(), OPEN_WITH_LOCAL_APP->shortcut().toString()));
  LOCATE_IMAGEHOST->setToolTip(
      PreferenceSettings().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v).toString());

  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setShortcutVisibleInContextMenu(true);

  RESIZE_ROWS_TO_CONTENT->setCheckable(true);
  RESIZE_ROWS_TO_CONTENT->setChecked(false);
  RESIZE_ROWS_TO_CONTENT->setToolTip(QString("<b>%0 (%1)</b><br/> Resize row to content when enabled. row height interactive when disabled")
                                         .arg(RESIZE_ROWS_TO_CONTENT->text(), RESIZE_ROWS_TO_CONTENT->shortcut().toString()));

  SHOW_PERFORMER_MANAGER->setCheckable(true);
  SHOW_PERFORMER_MANAGER->setChecked(
      PreferenceSettings().value(MemoryKey::SHOW_PERFORMERS_MANAGER_DATABASE.name, MemoryKey::SHOW_PERFORMERS_MANAGER_DATABASE.v).toBool());

  const QString& heightStr =
      QString::number(PreferenceSettings().value(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.v).toInt());
  CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT->setToolTip(heightStr);

  STRETCH_DETAIL_SECTION->setCheckable(true);
  STRETCH_DETAIL_SECTION->setChecked(
      PreferenceSettings().value(MemoryKey::PERFORMER_STRETCH_LAST_SECTION.name, MemoryKey::PERFORMER_STRETCH_LAST_SECTION.v).toBool());
  HORIZONTAL_HEADER_AGS->addAction(HIDE_THIS_COLUMN);
  HORIZONTAL_HEADER_AGS->addAction(SHOW_ALL_COLUMNS);
  HORIZONTAL_HEADER_AGS->addAction(STRETCH_DETAIL_SECTION);
  HORIZONTAL_HEADER_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  VERTICAL_HEADER_AGS->addAction(DELETE_RECORDS);
  VERTICAL_HEADER_AGS->addAction(RESIZE_ROWS_TO_CONTENT);
  VERTICAL_HEADER_AGS->addAction(RESIZE_ROWS_DEFAULT_SECTION_SIZE);
  VERTICAL_HEADER_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
}

QMenuBar* PerformersManagerActions::GetMenuBar() const {
  auto* fileMenu = new QMenu("File");
  fileMenu->addActions({LOAD_FROM_PJSON_PATH, LOAD_FROM_FILE_SYSTEM_STRUCTURE, LOAD_FROM_PERFORMERS_LIST});
  fileMenu->addSeparator();
  fileMenu->addActions({DUMP_ALL_RECORDS_INTO_PJSON_FILE, DUMP_SELECTED_RECORDS_INTO_PJSON_FILE});
  fileMenu->addSeparator();
  fileMenu->addActions({REFRESH_ALL_RECORDS_VIDS, REFRESH_SELECTED_RECORDS_VIDS});
  fileMenu->addSeparator();
  fileMenu->addAction(OPEN_WITH_LOCAL_APP);
  fileMenu->addSeparator();
  fileMenu->addAction(LOCATE_IMAGEHOST);
  fileMenu->setToolTipsVisible(true);

  auto* editMenu = new QMenu("Edit");
  editMenu->addActions({INIT_DATABASE, INIT_TABLE, INSERT_INTO_TABLE});
  editMenu->addSeparator();
  editMenu->addActions({DELETE_TABLE, DROP_TABLE});
  editMenu->addSeparator();
  editMenu->addAction(SUBMIT);
  editMenu->setToolTipsVisible(true);

  auto* viewMenu = new QMenu("View");
  viewMenu->addActions({COLUMNS_VISIBILITY, SHOW_ALL_COLUMNS});
  viewMenu->addSeparator();
  viewMenu->addActions({STRETCH_DETAIL_SECTION, RESIZE_ROWS_TO_CONTENT, RESIZE_ROWS_DEFAULT_SECTION_SIZE});
  viewMenu->addSeparator();
  viewMenu->addActions({CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT});
  viewMenu->setToolTipsVisible(true);

  auto* m_menuBar = new QMenuBar;
  m_menuBar->addMenu(fileMenu);
  m_menuBar->addMenu(editMenu);
  m_menuBar->addMenu(viewMenu);
  return m_menuBar;
}

PerformersManagerActions& g_performersManagerActions() {
  static PerformersManagerActions performersManagerActIns;
  return performersManagerActIns;
}
