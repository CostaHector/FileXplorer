#include "RibbonMovieDB.h"
#include "Actions/MovieDBActions.h"
#include "Actions/CastDBActions.h"
#include "Actions/TorrDBAction.h"
#include "Actions/ViewActions.h"
#include "public/PublicVariable.h"
#include "public/PublicMacro.h"

RibbonMovieDB::RibbonMovieDB(const QString& title, QWidget* parent)  //
    : QToolBar(title, parent)                                            //
{
  m_dbControlTB = new (std::nothrow) QToolBar("DB Control", this);
  CHECK_NULLPTR_RETURN_VOID(m_dbControlTB);
  m_dbControlTB->addActions(g_dbAct().DB_CONTROL_ACTIONS->actions());
  m_dbControlTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_dbControlTB->setOrientation(Qt::Orientation::Horizontal);

  m_extraFunctionTB = new (std::nothrow) QToolBar("Video DB Extra Function", this);
  CHECK_NULLPTR_RETURN_VOID(m_extraFunctionTB);
  m_extraFunctionTB->addActions(g_dbAct().EX_FUNCTION_ACTIONS->actions());
  m_extraFunctionTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_extraFunctionTB->setOrientation(Qt::Orientation::Vertical);
  m_extraFunctionTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  m_extraFunctionTB->setStyleSheet("QToolBar { max-width: 256px; }");

  m_functionsTB = new (std::nothrow) QToolBar("Function", this);
  CHECK_NULLPTR_RETURN_VOID(m_functionsTB);
  m_functionsTB->addAction(g_dbAct()._SUM);
  m_functionsTB->addAction(g_dbAct()._COUNT);
  m_functionsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  m_functionsTB->setOrientation(Qt::Orientation::Vertical);
  m_functionsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_24, IMAGE_SIZE::TABS_ICON_IN_MENU_24));
  m_functionsTB->setStyleSheet("QToolBar { max-width: 256px; }");

  m_dbViewHideShowTB = new (std::nothrow) QToolBar("Show/Hide Database View", this);
  CHECK_NULLPTR_RETURN_VOID(m_dbViewHideShowTB);
  m_dbViewHideShowTB->addAction(g_dbAct().QUICK_WHERE_CLAUSE);
  m_dbViewHideShowTB->addSeparator();
  m_dbViewHideShowTB->addAction(g_viewActions()._MOVIE_VIEW);
  m_dbViewHideShowTB->addSeparator();
  m_dbViewHideShowTB->addAction(g_castAct().PERFORMERS_BOOK);
  m_dbViewHideShowTB->addSeparator();
  m_dbViewHideShowTB->addAction(g_torrentsManagerActions().SHOW_TORRENTS_MANAGER);
  m_dbViewHideShowTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_dbViewHideShowTB->setOrientation(Qt::Orientation::Horizontal);

  addWidget(m_dbControlTB);
  addSeparator();
  addWidget(m_extraFunctionTB);
  addSeparator();
  addWidget(m_functionsTB);
  addSeparator();
  addWidget(m_dbViewHideShowTB);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RibbonMovieDB dbTB("Database Toolbar");
  dbTB.show();
  return a.exec();
}
#endif
