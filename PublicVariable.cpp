#include "PublicVariable.h"

auto TextReader(const QString& textPath) -> QString {
  QFile file(textPath);
  if (not file.exists()) {
    qDebug("File[%s] not found", qPrintable(textPath));
    return "";
  }
  if (not file.open(QIODevice::ReadOnly)) {
    qDebug("File[%s] open for read failed", qPrintable(textPath));
    return "";
  }
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  QString contents(stream.readAll());
  file.close();
  return contents;
}

namespace HEADERVIEW_SORT_INDICATOR_ORDER {
QString SortOrderEnum2String(const Qt::SortOrder orderEnum) {
  if (string2SortOrderEnumListTable.isEmpty()) {
    qDebug("[Error] string2SortOrderListTable is empty");
    return "";
  }
  for (auto it = string2SortOrderEnumListTable.cbegin(); it != string2SortOrderEnumListTable.cend(); ++it) {
    if (it.value() == orderEnum) {
      return it.key();
    }
  }
  return string2SortOrderEnumListTable.cbegin().key();
}
}  // namespace HEADERVIEW_SORT_INDICATOR_ORDER

const char* SUBMIT_BTN_STYLE =
    "QPushButton{"
    "    color: #fff;"
    "    background-color: DodgerBlue;"
    "    border-color: DodgerBlue;"
    "}"
    "QPushButton:hover {"
    "    color: #fff;"
    "    background-color: rgb(36, 118, 199);"
    "    border-color: rgb(36, 118, 199);"
    "}";

bool VerifyOneFilePath(const QString& fileKey) {
  // fileKey:
  // WIN32_PERFORMERS_TABLE
  // WIN32_AKA_PERFORMERS
  // WIN32_STANDARD_STUDIO_NAME
  // LINUX_PERFORMERS_TABLE
  // LINUX_AKA_PERFORMERS
  // LINUX_STANDARD_STUDIO_NAME
  constexpr int SYS_TYPE_PREFIX_LEN = 6;
  const QString relPath = QString("%1/bin/%2.txt").arg(PROJECT_PATH, fileKey.mid(SYS_TYPE_PREFIX_LEN));
  if (not PreferenceSettings().contains(fileKey)) {
    PreferenceSettings().setValue(fileKey, QFileInfo(relPath).absoluteFilePath());
  }
  QString savedPath = PreferenceSettings().value(fileKey).toString();
  if (QFile::exists(savedPath)) {
    return true;
  }
  QString refreshPath = QFileInfo(relPath).absoluteFilePath();
  if (QFile::exists(refreshPath)) {
    PreferenceSettings().setValue(fileKey, refreshPath);
    return true;
  }
  qFatal("aka %s file[%s] not found.", qPrintable(fileKey), qPrintable(refreshPath));
  return false;
}

bool VerifyOneFolderPath(const QString& fileKey) {
  // fileKey:
  // WIN32_RUNLOG
  // LINUX_RUNLOG
  constexpr int SYS_TYPE_PREFIX_LEN = 6;
  const QString relPath = QString("%1/bin/%2").arg(PROJECT_PATH, fileKey.mid(SYS_TYPE_PREFIX_LEN));
  if (not PreferenceSettings().contains(fileKey)) {
    PreferenceSettings().setValue(fileKey, QFileInfo(relPath).absoluteFilePath());
  }
  QString savedPath = PreferenceSettings().value(fileKey).toString();
  if (QFile::exists(savedPath)) {
    return true;
  }
  QString refreshPath = QFileInfo(relPath).absoluteFilePath();
  if (QFile::exists(refreshPath)) {
    PreferenceSettings().setValue(fileKey, refreshPath);
    return true;
  }
  qFatal("aka %s folder[%s] not found.", qPrintable(fileKey), qPrintable(refreshPath));
  return false;
}

bool InitOutterPlainTextPath() {
  bool initOk = true;
#ifdef _WIN32
  initOk = VerifyOneFilePath(MemoryKey::WIN32_AKA_PERFORMERS.name) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_PERFORMERS_TABLE.name) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_STANDARD_STUDIO_NAME.name) and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::WIN32_RUNLOG.name) and initOk;
#else
  initOk = VerifyOneFilePath(MemoryKey::LINUX_AKA_PERFORMERS.name) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::LINUX_PERFORMERS_TABLE.name) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::LINUX_STANDARD_STUDIO_NAME.name) and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::LINUX_RUNLOG.name) and initOk;
#endif
  return initOk;
}
