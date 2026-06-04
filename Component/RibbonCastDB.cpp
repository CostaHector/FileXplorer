#include "RibbonCastDB.h"
#include "CastDBActions.h"
#include "ViewActions.h"
#include "PublicMacro.h"
#include "ImageTool.h"
#include "StyleSheet.h"

RibbonCastDB::RibbonCastDB(const QString& title, QWidget* parent)  //
  : QToolBar{title, parent}                                      //
{
  auto& inst = CastDBActions::GetInst();

  m_BasicTableOp = new (std::nothrow) QToolBar{"Basic Table Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_BasicTableOp);
  m_BasicTableOp->addActions(inst.BASIC_TABLE_OP->actions());
  m_BasicTableOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_BasicTableOp->setOrientation(Qt::Orientation::Horizontal);

  m_SyncVidsFromDbOp = new (std::nothrow) QToolBar{"Export to Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_SyncVidsFromDbOp);
  m_SyncVidsFromDbOp->addAction(inst.SYNC_ALL_RECORDS_IMGS_FROM_DISK);
  m_SyncVidsFromDbOp->addAction(inst.SYNC_ALL_RECORDS_VIDS_FROM_DB);
  m_SyncVidsFromDbOp->addAction(inst.DUMP_ALL_RECORDS_INTO_PSON_FILE);
  m_SyncVidsFromDbOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_SyncVidsFromDbOp->setOrientation(Qt::Orientation::Vertical);
  m_SyncVidsFromDbOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(m_SyncVidsFromDbOp->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* castAppendToolbutton = inst.GetAppendCastToolButton(this);

  addAction(ViewActions::GetInst()._CAST_VIEW);
  addSeparator();
  addAction(inst._MODEL_SUBMIT_ALL);
  addAction(inst._MODEL_REPOPULATE);
  addSeparator();
  addWidget(castAppendToolbutton);
  addWidget(m_BasicTableOp);
  addSeparator();
  addWidget(m_SyncVidsFromDbOp);
  addSeparator();
  addAction(inst.OPEN_DB_WITH_LOCAL_APP);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}
