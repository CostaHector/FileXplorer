#include "PerformersManagerActions.h"
#include <QMenu>
#include <QActionGroup>
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

PerformersManagerActions::PerformersManagerActions(QObject* parent)
    : QObject{parent},
      REFRESH_ALL_RECORDS_VIDS(new(std::nothrow) QAction(tr("Refresh all record(s) vid"), this)),
      REFRESH_SELECTED_RECORDS_VIDS(new(std::nothrow) QAction(tr("Refresh selected record(s) vid"), this)),
      OPEN_RECORD_IN_FILE_SYSTEM(new(std::nothrow) QAction(tr("Open record"), this)),
      LOAD_FROM_PJSON_PATH(new(std::nothrow) QAction(tr("Load from pjson"), this)),
      LOAD_FROM_FILE_SYSTEM_STRUCTURE(new(std::nothrow) QAction(QIcon(":img/FOLDER_OPEN"), tr("Load from file-system structure"), this)),
      LOAD_FROM_PERFORMERS_LIST(new(std::nothrow) QAction(tr("Load from performers list"), this)),
      DUMP_ALL_RECORDS_INTO_PJSON_FILE(new(std::nothrow) QAction(tr("Dump all record(s)=>.pjson"), this)),
      DUMP_SELECTED_RECORDS_INTO_PJSON_FILE(new(std::nothrow) QAction(tr("Dump selected record(s)=>.pjson"), this)),
      OPEN_WITH_LOCAL_APP(new(std::nothrow) QAction(QIcon(":img/SQLITE_APP"), tr("Open with"), this)),
      LOCATE_IMAGEHOST(new(std::nothrow) QAction(tr("Locate imagehost"), this)),
      INIT_DATABASE(new(std::nothrow) QAction(QString("Init Database [%1]").arg(SystemPath::PEFORMERS_DATABASE), this)),
      INIT_TABLE(new(std::nothrow) QAction(QString("CREATE table [%1]").arg(DB_TABLE::PERFORMERS), this)),
      INSERT_INTO_TABLE(new(std::nothrow) QAction(QString("INSERT into table [%1]").arg(DB_TABLE::PERFORMERS), this)),
      DELETE_TABLE(new(std::nothrow) QAction("Clear records", this)),
      DROP_TABLE(new(std::nothrow) QAction("DROP the table", this)),
      SUBMIT(new(std::nothrow) QAction(QIcon(":img/SUBMIT"), "Submit", this)),

      CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT(new(std::nothrow) QAction(tr("Change performer image fixed height(px)"), this)),

      HORIZONTAL_HEADER_AGS(new(std::nothrow) QActionGroup(this)),

      DELETE_RECORDS(new(std::nothrow) QAction(tr("delete record(s)"), this)),
      VERTICAL_HEADER_AGS(new(std::nothrow) QActionGroup(this)),

      PERFORMERS_BOOK{new(std::nothrow) QAction(QIcon(":img/PERFORMERS_APP"), tr("Performers book"), this)} {
  LOAD_FROM_PJSON_PATH->setToolTip(QString("<b>%1 (%2)</b><br/> Load *.pjson from ImageHost.<br/>Update each column value by new one if primary key conflicts.")
                                       .arg(LOAD_FROM_PJSON_PATH->text(), LOAD_FROM_PJSON_PATH->shortcut().toString()));

  OPEN_WITH_LOCAL_APP->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O));
  OPEN_WITH_LOCAL_APP->setToolTip(QString("<b>%1 (%2)</b><br/> Open *.db file in local app.<br/>DB Browser(sqlite) and set it open db by default required.")
                                      .arg(OPEN_WITH_LOCAL_APP->text(), OPEN_WITH_LOCAL_APP->shortcut().toString()));
  LOCATE_IMAGEHOST->setToolTip(PreferenceSettings().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v).toString());

  SUBMIT->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  SUBMIT->setShortcutVisibleInContextMenu(true);

  PERFORMERS_BOOK->setCheckable(true);

  const QString& heightStr = QString::number(PreferenceSettings().value(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.v).toInt());
  CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT->setToolTip(heightStr);

  HORIZONTAL_HEADER_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  VERTICAL_HEADER_AGS->addAction(DELETE_RECORDS);
  VERTICAL_HEADER_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
}

QMenuBar* PerformersManagerActions::GetMenuBar() const {
  auto* fileMenu = new QMenu(tr("File"));
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

QMenu* PerformersManagerActions::GetRightClickMenu() const {
  auto* m_performerTableMenu = new QMenu(tr("performer table right click menu"));
  m_performerTableMenu->addAction(g_performersManagerActions().REFRESH_SELECTED_RECORDS_VIDS);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_performersManagerActions().OPEN_RECORD_IN_FILE_SYSTEM);
  m_performerTableMenu->addSeparator();
  m_performerTableMenu->addAction(g_performersManagerActions().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE);
  m_performerTableMenu->setToolTipsVisible(true);
  return m_performerTableMenu;
}

QActionGroup* PerformersManagerActions::GetVerAGS() const {
  return g_performersManagerActions().VERTICAL_HEADER_AGS;
}
QActionGroup* PerformersManagerActions::GetHorAGS() const {
  return g_performersManagerActions().HORIZONTAL_HEADER_AGS;
}

PerformersManagerActions& g_performersManagerActions() {
  static PerformersManagerActions performersManagerActIns;
  return performersManagerActIns;
}
