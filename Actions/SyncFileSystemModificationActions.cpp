#include "SyncFileSystemModificationActions.h"
#include "Component/SyncModifiyFileSystem.h"
#include "PublicVariable.h"

SyncFileSystemModificationActions& g_syncFileSystemModificationActions() {
  static SyncFileSystemModificationActions ins;
  return ins;
}

SyncFileSystemModificationActions::SyncFileSystemModificationActions(QObject* parent) {
  _SYNC_MOD_SWITCH->setCheckable(true);
  _SYNC_REVERSE_SWITCH->setCheckable(true);

  _SYNC_MOD_SWITCH->setToolTip("Modification on basic path will also syncronized to destination path,"
      "including\n1.file rename\n2.file move/into into\n3.file recycle\n4.file delete\nWarning: When copy items out, only first commands will succeed.");
  _SYNC_REVERSE_SWITCH->setToolTip("Also sync modification reverse back to basic path");
}

QToolBar* SyncFileSystemModificationActions::GetSyncSwitchToolbar() {
  const bool syncSwOn = PreferenceSettings().value("SYNC_FS_MOD", SyncModifiyFileSystem::m_syncModifyFileSystemSwitch).toBool();
  const bool syncBackOn = PreferenceSettings().value("SYNC_REVERSE_BACK", SyncModifiyFileSystem::m_alsoSyncReversebackSwitch).toBool();
  _SYNC_MOD_SWITCH->setChecked(syncSwOn);
  _SYNC_REVERSE_SWITCH->setChecked(syncBackOn);

  QToolBar* syncTb = new QToolBar("Sync Switch");
  syncTb->addAction(_SYNC_MOD_SWITCH);
  return syncTb;
}

QToolBar* SyncFileSystemModificationActions::GetSyncPathToolbar() {
  const QString& basicPath = PreferenceSettings().value("SYNC_BASIC_PATH", "").toString();
  const QString& toPath = PreferenceSettings().value("SYNC_TO_PATH", "").toString();

  QToolBar* syncTb = new QToolBar("Sync Path Widget");
  _BASIC_PATH = new QLineEdit(basicPath, syncTb);
  _BASIC_PATH->setToolTip("Basic path");
  _SYNC_TO_PATH = new QLineEdit(toPath, syncTb);
  _SYNC_TO_PATH->setToolTip("Modification synchronized to destination path");

  syncTb->addAction(_SYNC_REVERSE_SWITCH);
  syncTb->addWidget(_BASIC_PATH);
  syncTb->addWidget(_SYNC_TO_PATH);

  bool syncSwOn = _SYNC_MOD_SWITCH->isChecked();
  _BASIC_PATH->setEnabled(syncSwOn);
  _SYNC_TO_PATH->setEnabled(syncSwOn);
  _SYNC_REVERSE_SWITCH->setEnabled(syncSwOn);
  return syncTb;
}
