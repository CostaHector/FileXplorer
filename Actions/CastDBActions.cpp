#include "CastDBActions.h"
#include "PublicCheckMacro.h"
#include "ImageTool.h"
#include "Configuration.h"
#include "CastKey.h"
#include <QToolBar>
#include <QActionGroup>

CastDBActions& CastDBActions::GetInst() {
  static CastDBActions CastManagerActIns;
  return CastManagerActIns;
}

CastDBActions::CastDBActions(QObject* parent)  //
  : QObject{parent}                          //
{
  _MODEL_PENDING_CHANGES_ACTION = PendingChangesAction::GetDefault(this);
  CHECK_NULLPTR_RETURN_VOID(_MODEL_PENDING_CHANGES_ACTION);

  INSERT_A_ROW_BEFORE_CURRENT_INDEX = new (std::nothrow) QAction{QIcon(":img/INSERT_INTO_TABLE"), tr("Insert a row"), this};
  CHECK_NULLPTR_RETURN_VOID(INSERT_A_ROW_BEFORE_CURRENT_INDEX);
  INSERT_A_ROW_BEFORE_CURRENT_INDEX->setToolTip("Insert a row before current index in the DbTableView");

  APPEND_FROM_PSON_FILES = new (std::nothrow) QAction{tr("Append from pson files"), this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_PSON_FILES);
  APPEND_FROM_PSON_FILES->setToolTip(
      QString("<b>%1 (%2)</b><br/>Append casts records from *.pson files under ${ImageHost}. Override if primary key conflict")
          .arg(APPEND_FROM_PSON_FILES->text(), APPEND_FROM_PSON_FILES->shortcut().toString()));
  APPEND_FROM_FILE_SYSTEM_STRUCTURE = new (std::nothrow) QAction{tr("Append from file-system structure"), this};
  CHECK_NULLPTR_RETURN_VOID(APPEND_FROM_FILE_SYSTEM_STRUCTURE);
  APPEND_FROM_FILE_SYSTEM_STRUCTURE->setToolTip("Append casts records from file-system structure under ${ImageHost}");

  DELETE_RECORDS = new (std::nothrow) QAction{QIcon{":img/DELETE_FROM_TABLE"}, tr("Delete selected"), this};
  CHECK_NULLPTR_RETURN_VOID(DELETE_RECORDS);
  INIT_DATABASE = new (std::nothrow) QAction{QIcon{":img/CREATE_DATABASE"}, tr("Init Database"), this};
  CHECK_NULLPTR_RETURN_VOID(INIT_DATABASE);
  INIT_TABLE = new (std::nothrow) QAction{QIcon{":img/CREATE_TABLE"}, tr("Create table"), this};
  CHECK_NULLPTR_RETURN_VOID(INIT_TABLE);
  DELETE_TABLE = new (std::nothrow) QAction{QIcon{":img/DELETE_TABLE"}, tr("Delete table"), this};
  CHECK_NULLPTR_RETURN_VOID(DELETE_TABLE);
  DROP_TABLE = new (std::nothrow) QAction{QIcon{":img/DROP_TABLE"}, tr("Drop table"), this};
  CHECK_NULLPTR_RETURN_VOID(DROP_TABLE);
  BASIC_TABLE_OP = new (std::nothrow) QActionGroup(this);
  CHECK_NULLPTR_RETURN_VOID(BASIC_TABLE_OP);
  BASIC_TABLE_OP->addAction(DELETE_RECORDS);
  BASIC_TABLE_OP->addAction(INIT_DATABASE);
  BASIC_TABLE_OP->addAction(INIT_TABLE);
  BASIC_TABLE_OP->addAction(DELETE_TABLE);
  BASIC_TABLE_OP->addAction(DROP_TABLE);

  SYNC_SELECTED_RECORDS_IMGS_FROM_DISK = new (std::nothrow) QAction{QIcon{":img/SYNC_FROM_DISK"}, tr("Sync imgs field"), this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_SELECTED_RECORDS_IMGS_FROM_DISK);
  SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->setShortcut(QKeySequence(Qt::Key::Key_F5));
  SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->setToolTip(
      QString{"<b>%1 (%2)</b><br/> Sync selected record(s) Imgs field from disk"}  //
          .arg(SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->text(), SYNC_SELECTED_RECORDS_IMGS_FROM_DISK->shortcut().toString()));
  SYNC_ALL_RECORDS_IMGS_FROM_DISK = new (std::nothrow) QAction{QIcon{":img/SYNC_FROM_DISK"}, tr("Sync all imgs field"), this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_ALL_RECORDS_IMGS_FROM_DISK);
  SYNC_ALL_RECORDS_IMGS_FROM_DISK->setToolTip("Sync all record(s) Imgs field from disk");

  SYNC_SELECTED_RECORDS_VIDS_FROM_DB = new (std::nothrow) QAction{QIcon{":img/SYNC_FROM_DB"}, tr("Sync vids field"), this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_SELECTED_RECORDS_VIDS_FROM_DB);
  SYNC_SELECTED_RECORDS_VIDS_FROM_DB->setToolTip("Sync selected record(s) Vids field from db");
  SYNC_ALL_RECORDS_VIDS_FROM_DB = new (std::nothrow) QAction{QIcon{":img/SYNC_FROM_DB"}, tr("Sync all vids field"), this};
  CHECK_NULLPTR_RETURN_VOID(SYNC_ALL_RECORDS_VIDS_FROM_DB);
  SYNC_ALL_RECORDS_VIDS_FROM_DB->setToolTip("Sync all record(s) Vids field from db");

  m_showCastsTableAct = new QAction(QIcon{":img/DISPLAY_DETAIL_INFOMATIONS"}, tr("Table View"), this);
  CHECK_NULLPTR_RETURN_VOID(m_showCastsTableAct);
  m_showCastsTableAct->setCheckable(true);
  m_showCastsTableAct->setChecked(true);
  m_showCastsCardsListAct = new QAction(QIcon{":img/SHOW_CASTS_IN_CARDS_LIST"}, tr("Card View"), this);
  CHECK_NULLPTR_RETURN_VOID(m_showCastsCardsListAct);
  m_showCastsCardsListAct->setCheckable(true);
  m_showCastsCardsListAct->setChecked(getConfig(CastKey::SHOW_CASTS_IN_CARD_VIEW).toBool());

  OPEN_DB_WITH_LOCAL_APP = new (std::nothrow) QAction{QIcon(":img/SQLITE_APP"), tr("Open Database"), this};
  CHECK_NULLPTR_RETURN_VOID(OPEN_DB_WITH_LOCAL_APP);
  OPEN_DB_WITH_LOCAL_APP->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_O));
  OPEN_DB_WITH_LOCAL_APP->setToolTip(
      QString("<b>%1 (%2)</b><br/> Open *.db file in local app(DB Browser sqlite). Precondition: It has been set as default app")
          .arg(OPEN_DB_WITH_LOCAL_APP->text(), OPEN_DB_WITH_LOCAL_APP->shortcut().toString()));

  DUMP_SELECTED_RECORDS_INTO_PSON_FILE = new (std::nothrow) QAction{QIcon{":img/DUMP_INTO_PSON_FILE"}, tr("Dump records"), this};
  CHECK_NULLPTR_RETURN_VOID(DUMP_SELECTED_RECORDS_INTO_PSON_FILE);
  DUMP_SELECTED_RECORDS_INTO_PSON_FILE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key::Key_S));
  DUMP_SELECTED_RECORDS_INTO_PSON_FILE->setToolTip(
      QString{"<b>%1 (%2)</b><br/> Dump selected records to its pson file"}  //
          .arg(DUMP_SELECTED_RECORDS_INTO_PSON_FILE->text(), DUMP_SELECTED_RECORDS_INTO_PSON_FILE->shortcut().toString()));
  DUMP_ALL_RECORDS_INTO_PSON_FILE = new (std::nothrow) QAction{QIcon{":img/DUMP_INTO_PSON_FILE"}, tr("Dump all records"), this};
  CHECK_NULLPTR_RETURN_VOID(DUMP_ALL_RECORDS_INTO_PSON_FILE);
  DUMP_ALL_RECORDS_INTO_PSON_FILE->setToolTip("Dump all records to its pson file");
}

#include "MenuToolButton.h"
QWidget* CastDBActions::GetAppendCastToolButton(QWidget* notNullParent) {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  QList<QAction*> acts{INSERT_A_ROW_BEFORE_CURRENT_INDEX, APPEND_FROM_PSON_FILES, APPEND_FROM_FILE_SYSTEM_STRUCTURE};
  MenuToolButton* insertToolButton = new (std::nothrow) MenuToolButton(acts,                           //
                                                                     QToolButton::InstantPopup,               //
                                                                     Qt::ToolButtonStyle::ToolButtonTextUnderIcon, //
                                                                     IMAGE_SIZE::TABS_ICON_IN_MENU_48,
                                                                     notNullParent);
  CHECK_NULLPTR_RETURN_NULLPTR(insertToolButton);
  insertToolButton->SetCaption(QIcon{":img/INSERT_INTO_TABLE"}, tr("Insert"), "");
  return insertToolButton;
}

QWidget* CastDBActions::GetCastsDisplayMethodToolBar(QWidget* notNullParent) {
  CHECK_NULLPTR_RETURN_NULLPTR(notNullParent);
  auto* displayViewToolBar = new (std::nothrow) QToolBar{"CastDBViewSplitterViewSwitch", notNullParent};
  displayViewToolBar->setOrientation(Qt::Orientation::Vertical);
  displayViewToolBar->addAction(m_showCastsTableAct);
  displayViewToolBar->addAction(m_showCastsCardsListAct);
  displayViewToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  displayViewToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  return displayViewToolBar;
}
