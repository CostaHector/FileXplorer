#ifndef SYNCFILESYSTEMMODIFICATIONACTIONS_H
#define SYNCFILESYSTEMMODIFICATIONACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>
#include <QLineEdit>
#include <QToolBar>

class SyncFileSystemModificationActions : public QObject
{
  Q_OBJECT
 public:
  explicit SyncFileSystemModificationActions(QObject *parent = nullptr);

  QToolBar* GetSyncToolbar();

  QLineEdit* _BASIC_PATH{nullptr};
  QLineEdit* _SYNC_TO_PATH{nullptr};
  QAction* _SYNC_MOD_SWITCH{new QAction("Sync switch", this)};
  QAction* _SYNC_REVERSE_SWITCH{new QAction("Sync reverse back switch", this)};
};

SyncFileSystemModificationActions& g_syncFileSystemModificationActions();

#endif // SYNCFILESYSTEMMODIFICATIONACTIONS_H
