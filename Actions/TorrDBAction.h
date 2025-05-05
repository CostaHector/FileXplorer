#ifndef TORRDBACTION_H
#define TORRDBACTION_H

#include <QAction>
#include <QMenuBar>
#include <QObject>

class TorrDBAction : public QObject {
  Q_OBJECT
 public:
  explicit TorrDBAction(QObject* parent = nullptr);

  QAction* OPEN_DB_WITH_LOCAL_APP;

  QAction* INIT_DATABASE;
  QAction* INIT_TABLE;
  QAction* INSERT_INTO_TABLE;

  QAction* DELETE_FROM_TABLE;
  QAction* DROP_TABLE;
  QAction* SUBMIT;

  QAction* SHOW_TORRENTS_MANAGER;

  QMenuBar* GetMenuBar() const;
};

TorrDBAction& g_torrentsManagerActions();

#endif  // TORRDBACTION_H
