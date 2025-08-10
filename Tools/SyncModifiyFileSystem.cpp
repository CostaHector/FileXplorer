#include "SyncModifiyFileSystem.h"
#include <QDir>
#include <QFileInfo>
#include "MemoryKey.h"

bool SyncModifiyFileSystem::m_syncOperationSw = false;
bool SyncModifiyFileSystem::m_syncBackSw = true;
QString SyncModifiyFileSystem::m_basicPath;
QString SyncModifiyFileSystem::m_syncToPath;

void SyncModifiyFileSystem::LoadFromMemory() {
  m_basicPath = PreferenceSettings().value("SYNC_BASIC_PATH", "").toString();
  m_syncToPath = PreferenceSettings().value("SYNC_TO_PATH", "").toString();
  m_syncOperationSw = PreferenceSettings().value("SYNC_FS_MOD", SyncModifiyFileSystem::m_syncOperationSw).toBool();
  m_syncBackSw = PreferenceSettings().value("SYNC_REVERSE_BACK", SyncModifiyFileSystem::m_syncBackSw).toBool();
}

void SyncModifiyFileSystem::SetSyncOperationSwitch(const bool bOn) {
  m_syncOperationSw = bOn;
  PreferenceSettings().setValue("SYNC_FS_MOD", bOn);
}
void SyncModifiyFileSystem::SetSyncReverseBackSwitch(const bool bReverse) {
  m_syncBackSw = bReverse;
  PreferenceSettings().setValue("SYNC_REVERSE_BACK", bReverse);
}

bool SyncModifiyFileSystem::operator()(QString& path) const {
  if (!m_syncOperationSw) {
    return false;
  }
  if (m_basicPath == m_syncToPath) {
    qWarning("Basic path[%s] equals to synchronized to path, no need sync", qPrintable(m_basicPath));
    return false;
  }
  // path can be "m_basicPath" or "m_basicPath/xxx"
  // path can alse be "m_syncToPath" or "m_syncToPath/xxx" if m_alsoSyncReversebackSwitch is true
  bool needSync{false};
  if (path == m_basicPath || path.startsWith(m_basicPath + '/')) {
    path.replace(0, m_basicPath.size(), m_syncToPath);
    needSync = true;
  } else if (m_syncBackSw) {
    if (path == m_syncToPath || path.startsWith(m_syncToPath + '/')) {
      path.replace(0, m_syncToPath.size(), m_basicPath);
      needSync = true;
    }
  }
  return needSync;
}

bool SyncModifiyFileSystem::SetBasicPath(const QString& basicPath) {
  PreferenceSettings().setValue("SYNC_BASIC_PATH", basicPath);
  const QFileInfo fi{basicPath};
  if (!fi.isDir()) {
    qWarning("Path[%s] is not a folder", qPrintable(basicPath));
    return false;
  }
  m_basicPath = fi.absoluteFilePath();
  return true;
}

bool SyncModifiyFileSystem::SetSynchronizedToPaths(const QString& syncToPath) {
  PreferenceSettings().setValue("SYNC_TO_PATH", syncToPath);
  const QFileInfo fi{syncToPath};
  if (!fi.isDir()) {
    qWarning("Path[%s] is not a folder", qPrintable(syncToPath));
    return false;
  }
  m_syncToPath = fi.absoluteFilePath();
  return true;
}
