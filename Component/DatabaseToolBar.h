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

class DatabaseToolBar : public QToolBar {
 public:
  QToolBar* dbControlTB;
  QToolBar* functionsTB;
  QToolBar* dbViewHideShow;

  DatabaseToolBar(const QString& title, QWidget* parent = nullptr)
      : QToolBar(parent),
        dbControlTB(GetDatabaseControlTB()),
        functionsTB(GetFunctionsTB()),
        dbViewHideShow(GetHideShowToolButton()) {
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
    showHideDB->addAction(g_dbAct().DB_VIEW_CLOSE_SHOW);
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
    functionsTB->setOrientation(Qt::Orientation::Vertical);
    functionsTB->addActions(g_dbAct().DB_FUNCTIONS->actions());
    return functionsTB;
  }
};

#endif  // DATABASETOOLBAR_H
