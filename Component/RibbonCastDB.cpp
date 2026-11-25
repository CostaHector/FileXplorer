#include "RibbonCastDB.h"
#include "CastDBActions.h"
#include "ViewActions.h"
#include "PublicCheckMacro.h"
#include "ImageTool.h"
#include "StyleSheet.h"

RibbonCastDB::RibbonCastDB(const QString& title, QWidget* parent)  //
  : QToolBar{title, parent}                                      //
{
  auto& inst = CastDBActions::GetInst();

  m_basicTableOp = new (std::nothrow) QToolBar{"Basic Table Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_basicTableOp);
  m_basicTableOp->addActions(inst.BASIC_TABLE_OP->actions());
  m_basicTableOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_basicTableOp->setOrientation(Qt::Orientation::Horizontal);

  m_syncFieldsValueOp = new (std::nothrow) QToolBar{"Export to Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_syncFieldsValueOp);
  m_syncFieldsValueOp->addAction(inst.SYNC_ALL_RECORDS_IMGS_FROM_DISK);
  m_syncFieldsValueOp->addAction(inst.SYNC_ALL_RECORDS_VIDS_FROM_DB);
  m_syncFieldsValueOp->addAction(inst.DUMP_ALL_RECORDS_INTO_PSON_FILE);
  m_syncFieldsValueOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_syncFieldsValueOp->setOrientation(Qt::Orientation::Vertical);
  m_syncFieldsValueOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(m_syncFieldsValueOp->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* dispViewWidget = inst.GetCastsDisplayMethodToolBar(this);
  auto* castAppendToolbutton = inst.GetAppendCastToolButton(this);

  addAction(ViewActions::GetInst()._CAST_VIEW);  // 显示/隐藏
  addWidget(dispViewWidget);
  addSeparator();
  addAction(inst._MODEL_PENDING_CHANGES_ACTION); // 数据库状态
  addSeparator();
  addWidget(castAppendToolbutton); // 插入/导出/基础功能
  addWidget(m_basicTableOp);
  addSeparator();
  addWidget(m_syncFieldsValueOp);  // 同步字段
  addSeparator();
  addAction(inst.OPEN_DB_WITH_LOCAL_APP); // 打开
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}
