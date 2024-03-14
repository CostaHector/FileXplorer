#ifndef LOGVIEWACTIONS_H
#define LOGVIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QObject>
#include <QToolBar>

class LogViewActions : public QObject {
  Q_OBJECT
 public:
  explicit LogViewActions(QObject* parent = nullptr) : QObject(parent) {
    LOG_TYPE_AGS->addAction(_ALL);
    LOG_TYPE_AGS->addAction(_INFO);
    LOG_TYPE_AGS->addAction(_DEBUG);
    LOG_TYPE_AGS->addAction(_WARNING);
    LOG_TYPE_AGS->addAction(_CRITICAL);
    LOG_TYPE_AGS->addAction(_FATAL);

    LOG_TYPE_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    foreach (QAction* act, LOG_TYPE_AGS->actions()) {
      act->setCheckable(true);
    }
    _ALL->setChecked(true);
  }

  QToolBar* getLogsTypeToolBar() const {
    auto* typeTB = new QToolBar;
    typeTB->addActions(LOG_TYPE_AGS->actions());
    return typeTB;
  }

  QMenu* getLogsTypeMenu() const {
    auto* typeMenu = new QMenu(tr("Log type menu"));
    typeMenu->setIcon(QIcon(":/themes/FILE_SYSTEM_FILTER"));
    typeMenu->addActions(LOG_TYPE_AGS->actions());
    return typeMenu;
  }

  QAction* _REFRESH_LOG{new QAction{QIcon(":/themes/RELOAD_FROM_DISK"), tr("Refresh"), this}};
  QAction* _COPY_LOCATE_INFO{new QAction{QIcon(":/themes/_LOCATE_WHERE"), tr("Copy location"), this}};
  QAction* _REVEAL_LOGS_FILE{new QAction{QIcon(":/themes/FLOW_LOGS"), tr("Reveal log file"), this}};

  QAction* _ALL{new QAction{QIcon(":/themes/_ALL"), "All", this}};
  QAction* _INFO{new QAction{QIcon(":/themes/_INFO"), "Info", this}};
  QAction* _DEBUG{new QAction{QIcon(":/themes/_DEBUG"), "Debug", this}};
  QAction* _WARNING{new QAction{QIcon(":/themes/_WARNING"), "Warning", this}};
  QAction* _CRITICAL{new QAction{QIcon(":/themes/_CRITICAL"), "Critical", this}};
  QAction* _FATAL{new QAction{QIcon(":/themes/_FATAL"), "Fatal", this}};
  QActionGroup* LOG_TYPE_AGS{new QActionGroup{this}};
};
LogViewActions& g_logAg();
#endif  // LOGVIEWACTIONS_H
