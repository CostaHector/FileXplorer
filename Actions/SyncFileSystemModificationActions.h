#ifndef SYNCFILESYSTEMMODIFICATIONACTIONS_H
#define SYNCFILESYSTEMMODIFICATIONACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QLineEdit>
#include <QObject>
#include <QToolBar>

class SyncFileSystemModificationActions : public QObject {
  Q_OBJECT
 public:
  explicit SyncFileSystemModificationActions(QObject* parent = nullptr);

  QToolBar* GetSyncSwitchToolbar();
  QToolBar* GetSyncPathToolbar();

  QLineEdit* _BASIC_PATH{nullptr};
  QLineEdit* _SYNC_TO_PATH{nullptr};
  QAction* _SYNC_MOD_SWITCH{new QAction(QIcon(":/themes/SYNC_MODIFICATION_SWITCH"), "Sync", this)};
  QAction* _SYNC_REVERSE_SWITCH{new QAction(QIcon(":/themes/SYNC_REVERSE_SWITCH"), "reverse back", this)};
};

SyncFileSystemModificationActions& g_syncFileSystemModificationActions();

#endif  // SYNCFILESYSTEMMODIFICATIONACTIONS_H
