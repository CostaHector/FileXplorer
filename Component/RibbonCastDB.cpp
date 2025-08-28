#include "RibbonCastDB.h"
#include "CastDBActions.h"
#include "ViewActions.h"
#include "PublicMacro.h"
#include "StyleSheet.h"

RibbonCastDB::RibbonCastDB(const QString& title, QWidget* parent)  //
  : QToolBar{title, parent}                                      //
{
  auto& inst = g_castAct();

  m_BasicTableOp = new (std::nothrow) QToolBar{"Basic Table Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_BasicTableOp);
  m_BasicTableOp->addActions(inst.BASIC_TABLE_OP->actions());
  m_BasicTableOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_BasicTableOp->setOrientation(Qt::Orientation::Horizontal);

  m_SyncImgsFromDbOp = new (std::nothrow) QToolBar{"Sync Imgs From Db Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_SyncImgsFromDbOp);
  m_SyncImgsFromDbOp->addActions(inst._SYNC_IMGS_OP->actions());
  m_SyncImgsFromDbOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_SyncImgsFromDbOp->setOrientation(Qt::Orientation::Vertical);
  m_SyncImgsFromDbOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  m_SyncImgsFromDbOp->setStyleSheet("QToolBar { max-width: 256px; }");

  m_SyncVidsFromDbOp = new (std::nothrow) QToolBar{"Sync Vids From Db Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_SyncVidsFromDbOp);
  m_SyncVidsFromDbOp->addActions(inst._SYNC_VIDS_OP->actions());
  m_SyncVidsFromDbOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_SyncVidsFromDbOp->setOrientation(Qt::Orientation::Vertical);
  m_SyncVidsFromDbOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  m_SyncVidsFromDbOp->setStyleSheet("QToolBar { max-width: 256px; }");

  m_ExportToOp = new (std::nothrow) QToolBar{"Export to Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_ExportToOp);
  m_ExportToOp->addActions(inst.EXPORT_OP->actions());
  m_ExportToOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_ExportToOp->setOrientation(Qt::Orientation::Vertical);
  m_ExportToOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  m_ExportToOp->setStyleSheet("QToolBar { max-width: 256px; }");

  auto* castAppendToolbutton = inst.GetAppendCastToolButton(this);

  addAction(g_viewActions()._CAST_VIEW);
  addSeparator();
  addAction(inst.SUBMIT);
  addSeparator();
  addWidget(castAppendToolbutton);
  addWidget(m_BasicTableOp);
  addSeparator();
  addWidget(m_SyncImgsFromDbOp);
  addWidget(m_SyncVidsFromDbOp);
  addWidget(m_ExportToOp);
  addSeparator();
  addAction(inst.OPEN_DB_WITH_LOCAL_APP);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}

// #define RUN_MAIN_FILE 1
#ifdef RUN_MAIN_FILE
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RibbonCastDB rcd{"Ribbon Cast"};
  rcd.show();
  a.exec();
  return 0;
}
#endif
