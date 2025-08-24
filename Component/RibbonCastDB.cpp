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

  m_RefreshOp = new (std::nothrow) QToolBar{"Refresh Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_RefreshOp);
  m_RefreshOp->addActions(inst.REFRESH_OP->actions());
  m_RefreshOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_RefreshOp->setOrientation(Qt::Orientation::Vertical);
  m_RefreshOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  m_RefreshOp->setStyleSheet("QToolBar { max-width: 256px; }");

  m_FileSystemOp = new (std::nothrow) QToolBar{"File System Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_FileSystemOp);
  m_FileSystemOp->addActions(inst.FILE_SYSTEM_OP->actions());
  m_FileSystemOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_FileSystemOp->setOrientation(Qt::Orientation::Vertical);
  m_FileSystemOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  m_FileSystemOp->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(m_FileSystemOp->layout(), Qt::AlignmentFlag::AlignLeft);

  m_LoadExtentOp = new (std::nothrow) QToolBar{"Load Extend Data Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_LoadExtentOp);
  m_LoadExtentOp->addActions(inst.LOAD_EXTEND_OP->actions());
  m_LoadExtentOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_LoadExtentOp->setOrientation(Qt::Orientation::Vertical);
  m_LoadExtentOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  m_LoadExtentOp->setStyleSheet("QToolBar { max-width: 256px; }");

  m_ExportToOp = new (std::nothrow) QToolBar{"Export to Operation", this};
  CHECK_NULLPTR_RETURN_VOID(m_ExportToOp);
  m_ExportToOp->addActions(inst.EXPORT_OP->actions());
  m_ExportToOp->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_ExportToOp->setOrientation(Qt::Orientation::Vertical);
  m_ExportToOp->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  m_ExportToOp->setStyleSheet("QToolBar { max-width: 256px; }");

  addAction(g_viewActions()._CAST_VIEW);
  addSeparator();
  addWidget(m_BasicTableOp);
  addSeparator();
  addWidget(m_RefreshOp);
  addSeparator();
  addWidget(m_FileSystemOp);
  addSeparator();
  addWidget(m_LoadExtentOp);
  addSeparator();
  addWidget(m_ExportToOp);
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
