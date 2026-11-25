#include "RibbonMovieDB.h"
#include "MovieDBActions.h"
#include "TorrDBAction.h"
#include "ViewActions.h"
#include "ImageTool.h"
#include "StyleSheet.h"
#include "PublicCheckMacro.h"

RibbonMovieDB::RibbonMovieDB(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent}                                        //
{
  auto& inst = MovieDBActions::GetInst();

  m_basicTableOp = new (std::nothrow) QToolBar{"DB Control", this};
  CHECK_NULLPTR_RETURN_VOID(m_basicTableOp);
  m_basicTableOp->addActions(inst.BASIC_TABLE_OP->actions());
  m_basicTableOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_basicTableOp->setOrientation(Qt::Orientation::Horizontal);

  m_extraFunctionTB = new (std::nothrow) QToolBar{"Video DB Extra Function", this};
  CHECK_NULLPTR_RETURN_VOID(m_extraFunctionTB);
  m_extraFunctionTB->addActions(inst.EX_FUNCTION_ACTIONS->actions());
  m_extraFunctionTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_extraFunctionTB->setOrientation(Qt::Orientation::Vertical);
  m_extraFunctionTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(m_extraFunctionTB->layout(), Qt::AlignmentFlag::AlignLeft);

  m_functionsTB = new (std::nothrow) QToolBar{"Function", this};
  CHECK_NULLPTR_RETURN_VOID(m_functionsTB);
  m_functionsTB->addAction(inst._SUM);
  m_functionsTB->addAction(inst._COUNT);
  m_functionsTB->addAction(inst._OPEN_DB_WITH_LOCAL_APP);
  m_functionsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_functionsTB->setOrientation(Qt::Orientation::Vertical);
  m_functionsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(m_functionsTB->layout(), Qt::AlignmentFlag::AlignLeft);

  m_studioTB = new (std::nothrow) QToolBar("Studio Edit Toolbar", this);
  CHECK_NULLPTR_RETURN_VOID(m_studioTB);
  m_studioTB->addAction(inst.SET_STUDIO);
  m_studioTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_studioTB->setOrientation(Qt::Orientation::Horizontal);

  m_castEditTB = new (std::nothrow) QToolBar("Cast Edit Toolbar", this);
  CHECK_NULLPTR_RETURN_VOID(m_castEditTB);
  m_castEditTB->addAction(inst.SET_CAST);
  m_castEditTB->addAction(inst.APPEND_CAST);
  m_castEditTB->addAction(inst.REMOVE_CAST);
  m_castEditTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_castEditTB->setOrientation(Qt::Orientation::Vertical);
  m_castEditTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(m_castEditTB->layout(), Qt::AlignmentFlag::AlignLeft);

  m_tagsEditTB = new (std::nothrow) QToolBar{"Tags Edit Toolbar", this};
  CHECK_NULLPTR_RETURN_VOID(m_tagsEditTB);
  m_tagsEditTB->addAction(inst.SET_TAGS);
  m_tagsEditTB->addAction(inst.APPEND_TAGS);
  m_tagsEditTB->addAction(inst.REMOVE_TAGS);
  m_tagsEditTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_tagsEditTB->setOrientation(Qt::Orientation::Vertical);
  m_tagsEditTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  SetLayoutAlightment(m_tagsEditTB->layout(), Qt::AlignmentFlag::AlignLeft);

  m_dbViewHideShowTB = new (std::nothrow) QToolBar{"Show/Hide Database View", this};
  CHECK_NULLPTR_RETURN_VOID(m_dbViewHideShowTB);
  m_dbViewHideShowTB->addAction(ViewActions::GetInst()._MOVIE_VIEW);
  m_dbViewHideShowTB->addSeparator();
  m_dbViewHideShowTB->addAction(g_torrActions().SHOW_TORRENTS_MANAGER);
  m_dbViewHideShowTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_dbViewHideShowTB->setOrientation(Qt::Orientation::Vertical);
  m_dbViewHideShowTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));


  addWidget(m_dbViewHideShowTB); // 显示/隐藏
  addSeparator();
  addAction(inst._MODEL_PENDING_CHANGES_ACTION); // 数据库状态
  addSeparator();
  addWidget(inst.GetScanFilesToolButton(this)); // 插入/导出/基础功能
  addAction(inst.EXPORT_TO_EFU_FILE);
  addWidget(m_basicTableOp);
  addSeparator();
  addWidget(m_extraFunctionTB); // 同步字段
  addSeparator();
  addWidget(m_functionsTB); // 统计， 打开
  addSeparator();
  addWidget(m_studioTB);   // 编辑
  addWidget(m_castEditTB);
  addWidget(m_tagsEditTB);
  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}
