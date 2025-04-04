#ifndef DATABASETOOLBAR_H
#define DATABASETOOLBAR_H

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>

#include "Actions/DataBaseActions.h"
#include "Actions/PerformersManagerActions.h"
#include "Actions/TorrentsManagerActions.h"
#include "Actions/ViewActions.h"
#include "PublicVariable.h"

class DatabaseToolBar : public QToolBar {
 public:
  QToolBar* dbControlTB;
  QToolBar* functionsTB;
  QToolBar* dbViewHideShow;

  DatabaseToolBar(const QString& title, QWidget* parent = nullptr)
      : QToolBar(title, parent), dbControlTB(GetDatabaseControlTB()), functionsTB(GetFunctionsTB()), dbViewHideShow(GetHideShowToolButton()) {
    addWidget(dbControlTB);
    addSeparator();
    addWidget(functionsTB);
    addSeparator();
    addWidget(dbViewHideShow);
  }

  auto GetHideShowToolButton() -> QToolBar* {
    QToolBar* showHideDB = new QToolBar("show/hide Database view", this);
    showHideDB->addAction(g_dbAct().QUICK_WHERE_CLAUSE);
    showHideDB->addSeparator();
    showHideDB->addAction(g_viewActions()._MOVIE_VIEW);
    showHideDB->addSeparator();
    showHideDB->addAction(g_performersManagerActions().SHOW_PERFORMER_MANAGER);
    showHideDB->addSeparator();
    showHideDB->addAction(g_torrentsManagerActions().SHOW_TORRENTS_MANAGER);
    showHideDB->addSeparator();
    showHideDB->setOrientation(Qt::Orientation::Horizontal);
    showHideDB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    return showHideDB;
  }

  auto GetDatabaseControlTB() -> QToolBar* {
    QToolBar* databaseControlTB = new QToolBar("Database control", this);
    databaseControlTB->addActions(g_dbAct().DB_CONTROL_ACTIONS->actions());
    databaseControlTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    return databaseControlTB;
  }

  auto GetFunctionsTB() -> QToolBar* {
    QToolBar* functionsTB = new QToolBar("Function", this);
    functionsTB->addAction(g_dbAct()._SUM);
    functionsTB->addAction(g_dbAct()._COUNT);
    functionsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    functionsTB->setOrientation(Qt::Orientation::Vertical);
    functionsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
    functionsTB->setStyleSheet("QToolBar { max-width: 256px; }");
    return functionsTB;
  }
};

#endif  // DATABASETOOLBAR_H
