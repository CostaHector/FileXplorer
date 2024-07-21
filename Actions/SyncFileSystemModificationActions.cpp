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

  _SYNC_MOD_SWITCH->setToolTip("Modification on basic will also works on syned to path");
  _SYNC_REVERSE_SWITCH->setToolTip("Also sync back");
}

QToolBar* SyncFileSystemModificationActions::GetSyncToolbar() {
  QToolBar* tb = new QToolBar("Sync");
  const QString& basicPath = PreferenceSettings().value("SYNC_BASIC_PATH", "").toString();
  const QString& toPath = PreferenceSettings().value("SYNC_TO_PATH", "").toString();
  const bool syncSwOn = PreferenceSettings().value("SYNC_FS_MOD", SyncModifiyFileSystem::m_syncModifyFileSystemSwitch).toBool();
  const bool syncBackOn = PreferenceSettings().value("SYNC_REVERSE_BACK", SyncModifiyFileSystem::m_alsoSyncReversebackSwitch).toBool();
  _SYNC_MOD_SWITCH->setChecked(syncSwOn);
  _SYNC_REVERSE_SWITCH->setChecked(syncBackOn);

  _BASIC_PATH = new QLineEdit(basicPath, tb);
  _BASIC_PATH->setPlaceholderText("basic path");
  _SYNC_TO_PATH = new QLineEdit(toPath, tb);
  _BASIC_PATH->setPlaceholderText("sync to path");
  tb->addAction(_SYNC_MOD_SWITCH);
  tb->addWidget(_BASIC_PATH);
  tb->addWidget(_SYNC_TO_PATH);

  _BASIC_PATH->setEnabled(syncSwOn);
  _SYNC_TO_PATH->setEnabled(syncSwOn);
  _SYNC_REVERSE_SWITCH->setEnabled(syncSwOn);
  return tb;
}
