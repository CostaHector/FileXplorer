#include "InitCheck.h"

namespace InitCheck{

bool VerifyOneFilePath(const KV& kv, const QString& fileType) {
  // fileKey:
  // WIN32_PERFORMERS_TABLE
  // WIN32_AKA_PERFORMERS
  // WIN32_STANDARD_STUDIO_NAME
  // LINUX_PERFORMERS_TABLE
  // LINUX_AKA_PERFORMERS
  // LINUX_STANDARD_STUDIO_NAME
  const QString& fileKey = kv.name;
  constexpr int SYS_TYPE_PREFIX_LEN = 6;
  const QString relPath = QString("%1/bin/%2.%3").arg(PROJECT_PATH, fileKey.mid(SYS_TYPE_PREFIX_LEN), fileType);
  if (not PreferenceSettings().contains(fileKey)) {
    PreferenceSettings().setValue(fileKey, QFileInfo(relPath).absoluteFilePath());
  }
  QString savedPath = PreferenceSettings().value(fileKey).toString();
  if (kv.checker(savedPath)) {
    return true;
  }
  QString refreshPath = QFileInfo(relPath).absoluteFilePath();
  if (kv.checker(refreshPath)) {
    PreferenceSettings().setValue(fileKey, refreshPath);
    return true;
  }
  qWarning("aka %s file[%s] not found.", qPrintable(fileKey), qPrintable(refreshPath));
  return false;
}

bool VerifyOneFolderPath(const KV& kv) {
  // fileKey:
  // WIN32_RUNLOG
  // LINUX_RUNLOG
  const QString& fileKey = kv.name;
  constexpr int SYS_TYPE_PREFIX_LEN = 6;
  const QString relPath = QString("%1/bin/%2").arg(PROJECT_PATH, fileKey.mid(SYS_TYPE_PREFIX_LEN));
  if (not PreferenceSettings().contains(fileKey)) {
    PreferenceSettings().setValue(fileKey, QFileInfo(relPath).absoluteFilePath());
  }
  QString savedPath = PreferenceSettings().value(fileKey).toString();
  if (kv.checker(savedPath)) {
    return true;
  }
  QString refreshPath = QFileInfo(relPath).absoluteFilePath();
  if (kv.checker(refreshPath)) {
    PreferenceSettings().setValue(fileKey, refreshPath);
    return true;
  }
  qWarning("aka %s folder[%s] not found.", qPrintable(fileKey), qPrintable(refreshPath));
  return false;
}

bool InitOutterPlainTextPath() {
  bool initOk = true;
#ifdef _WIN32
  initOk = VerifyOneFilePath(MemoryKey::WIN32_MEDIAINFO_LIB_PATH) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_AKA_PERFORMERS) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_PERFORMERS_TABLE) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_STANDARD_STUDIO_NAME, "json") and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH, "bat") and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::WIN32_RUNLOG) and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::WIN32_RUND_IMG_PATH) and initOk;
#else
  initOk = VerifyOneFilePath(MemoryKey::LINUX_MEDIAINFO_LIB_PATH) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::LINUX_AKA_PERFORMERS) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::LINUX_PERFORMERS_TABLE) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::LINUX_STANDARD_STUDIO_NAME, "json") and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::LINUX_RUNLOG) and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::LINUX_RUND_IMG_PATH) and initOk;
#endif
  return initOk;
}


}
