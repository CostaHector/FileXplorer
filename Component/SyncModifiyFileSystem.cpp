#include "SyncModifiyFileSystem.h"
#include <QDir>
#include <QFileInfo>
#include "PublicVariable.h"

bool SyncModifiyFileSystem::m_syncModifyFileSystemSwitch = false;
bool SyncModifiyFileSystem::m_alsoSyncReversebackSwitch = true;

QString SyncModifiyFileSystem::m_basicPath;
QString SyncModifiyFileSystem::m_synchronizedToPath;

SyncModifiyFileSystem::SyncModifiyFileSystem() {}

void SyncModifiyFileSystem::LoadFromMemory() {
  const QString& basicPath = PreferenceSettings().value("SYNC_BASIC_PATH", "").toString();
  const QString& toPath = PreferenceSettings().value("SYNC_TO_PATH", "").toString();
  const bool syncSwOn = PreferenceSettings().value("SYNC_FS_MOD", SyncModifiyFileSystem::m_syncModifyFileSystemSwitch).toBool();
  const bool syncBackOn = PreferenceSettings().value("SYNC_REVERSE_BACK", SyncModifiyFileSystem::m_alsoSyncReversebackSwitch).toBool();
  if (not basicPath.isEmpty()) {
    SetBasicPath(basicPath);
  }
  if (not toPath.isEmpty()) {
    SetSynchronizedToPaths(toPath);
  }
  SyncModifiyFileSystem::m_syncModifyFileSystemSwitch = syncSwOn;
  SyncModifiyFileSystem::m_alsoSyncReversebackSwitch = syncBackOn;
}
bool SyncModifiyFileSystem::operator()(QString& path) const {
  if (!m_syncModifyFileSystemSwitch) {
    return false;
  }
  if (m_basicPath == m_synchronizedToPath) {
    qWarning("Basic path[%s] equals to synchronized to path, no need sync", qPrintable(m_basicPath));
    return false;
  }
  bool needSync{false};
  if (path.startsWith(m_basicPath)) {
    path.replace(0, m_basicPath.size(), m_synchronizedToPath);
    needSync = true;
  } else if (m_alsoSyncReversebackSwitch) {
    if (path.startsWith(m_synchronizedToPath)) {
      path.replace(0, m_synchronizedToPath.size(), m_basicPath);
      needSync = true;
    }
  }
  return needSync;
}

bool SyncModifiyFileSystem::SetBasicPath(const QString& basicPath) {
  const QFileInfo fi{basicPath};
  if (!fi.isDir()) {
    qWarning("Path[%s] is not a folder", qPrintable(basicPath));
    return false;
  }
  m_basicPath = fi.absoluteFilePath();  // append a slash
  return true;
}

bool SyncModifiyFileSystem::SetSynchronizedToPaths(const QString& synchronizedToPath) {
  const QFileInfo fi{synchronizedToPath};
  if (!fi.isDir()) {
    qWarning("Path[%s] is not a folder", qPrintable(synchronizedToPath));
    return false;
  }
  m_synchronizedToPath = fi.absoluteFilePath();  // append a slash
  return true;
}
