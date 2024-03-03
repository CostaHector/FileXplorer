#ifndef TORRENTSMANAGERACTIONS_H
#define TORRENTSMANAGERACTIONS_H

#include <QAction>
#include <QMenuBar>
#include <QObject>

class TorrentsManagerActions : public QObject {
  Q_OBJECT
 public:
  explicit TorrentsManagerActions(QObject* parent = nullptr);

  QAction* OPEN_WITH_LOCAL_APP;

  QAction* INIT_DATABASE;
  QAction* INIT_TABLE;
  QAction* INSERT_INTO_TABLE;

  QAction* DELETE_FROM_TABLE;
  QAction* DROP_TABLE;
  QAction* SUBMIT;

  QAction* SHOW_TORRENTS_MANAGER;

  QMenuBar* GetMenuBar() const;
};

TorrentsManagerActions& g_torrentsManagerActions();

#endif  // TORRENTSMANAGERACTIONS_H
