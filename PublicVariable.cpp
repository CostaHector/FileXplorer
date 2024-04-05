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
  initOk = VerifyOneFilePath(MemoryKey::WIN32_AKA_PERFORMERS) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_PERFORMERS_TABLE) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_STANDARD_STUDIO_NAME, "json") and initOk;
  initOk = VerifyOneFilePath(MemoryKey::WIN32_TERMINAL_OPEN_BATCH_FILE_PATH, "bat") and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::WIN32_RUNLOG) and initOk;
#else
  initOk = VerifyOneFilePath(MemoryKey::LINUX_AKA_PERFORMERS) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::LINUX_PERFORMERS_TABLE) and initOk;
  initOk = VerifyOneFilePath(MemoryKey::LINUX_STANDARD_STUDIO_NAME, "json") and initOk;
  initOk = VerifyOneFolderPath(MemoryKey::LINUX_RUNLOG) and initOk;
#endif
  return initOk;
}

ValueChecker::ValueChecker(const QStringList& candidates, const VALUE_TYPE valueType_)
    : valueType{valueType_}, m_strCandidates{candidates.cbegin(), candidates.cend()} {}

ValueChecker::ValueChecker(int minV_, int maxV_) : valueType{RANGE_INT}, minV{minV_}, maxV{maxV_} {}

ValueChecker::ValueChecker(const QSet<QChar>& chars, int minLength) : valueType{SWITCH_STRING}, m_switchStates{chars}, m_switchMinCnt{minLength} {}

ValueChecker::ValueChecker(const VALUE_TYPE valueType_) : valueType(valueType_) {}

int ValueChecker::getFileExtensionDotIndex(const QString& path) {
  constexpr static int EXTENSION_SIZE = 5;
  const int lastDot = path.lastIndexOf('.');
  const int N = path.size();
  if (lastDot == -1) {
    return N;
  }
  if (N >= 2 and lastDot == N - 2 and path[N - 1].isDigit()) {
    return N;
  }
  if (N >= 3 and lastDot == N - 3 and path[N - 1].isDigit() and path[N - 2].isDigit()) {
    return N;
  }
  return N - (lastDot + 1) > EXTENSION_SIZE ? N : lastDot;
}

QString ValueChecker::GetFileExtension(const QString& path) {
  return path.mid(getFileExtensionDotIndex(path));
}

bool ValueChecker::isFileExist(const QString& path) {
  return QFileInfo(path).isFile();
}

bool ValueChecker::isFolderExist(const QString& path) {
  return QFileInfo(path).isDir();
}

bool ValueChecker::isStrInCandidate(const QString& str) const {
  return m_strCandidates.isEmpty() or m_strCandidates.contains(str);
}

bool ValueChecker::isSpecifiedExtensionFileExist(const QString& path) const {
  const QString& ext = GetFileExtension(path);
  return isFileExist(path) and not ext.isEmpty() and isStrInCandidate(ext);
}

bool ValueChecker::isIntInRange(const int v) const {
  return minV <= v and v < maxV;
}

bool ValueChecker::isSwitchString(const QString& switchs) const {
  return switchs.size() >= m_switchMinCnt and QSet<QChar>(switchs.cbegin(), switchs.cend()) == m_switchStates;
}

bool ValueChecker::operator()(const QVariant& v) const {
  switch (valueType) {
    case EXT_SPECIFIED_FILE_PATH:
      return isSpecifiedExtensionFileExist(v.toString());
    case CANDIDATE_STRING:
      return isStrInCandidate(v.toString());
    case FILE_PATH:
      return isFileExist(v.toString());
    case FOLDER_PATH:
      return isFolderExist(v.toString());
    case RANGE_INT:
      return isIntInRange(v.toInt());
    case SWITCH_STRING:
      return isSwitchString(v.toString());
    case PLAIN_STR:
    case PLAIN_INT:
    case PLAIN_BOOL:
    case PLAIN_FLOAT:
    case PLAIN_DOUBLE:
    case QSTRING_LIST:
    default:
      return true;
  }
}

QString ValueChecker::valueToString(const QVariant& v) const {
  switch (valueType) {
    case EXT_SPECIFIED_FILE_PATH:
    case CANDIDATE_STRING:
    case FILE_PATH:
    case FOLDER_PATH:
    case SWITCH_STRING:
    case PLAIN_STR:
      return v.toString();
    case RANGE_INT:
    case PLAIN_INT:
    case PLAIN_BOOL:
      return QString::number(v.toInt());
    case PLAIN_FLOAT:
      return QString::number(v.toFloat());
    case PLAIN_DOUBLE:
      return QString::number(v.toDouble());
    case QSTRING_LIST:
      return v.toStringList().join('\n');
    default:
      return "";
  }
}

QVariant ValueChecker::strToQVariant(const QString& v) const {
  switch (valueType) {
    case EXT_SPECIFIED_FILE_PATH:
    case CANDIDATE_STRING:
    case FILE_PATH:
    case FOLDER_PATH:
    case SWITCH_STRING:
    case PLAIN_STR:
      return v;
    case RANGE_INT:
    case PLAIN_INT:
      return v.toInt();
    case PLAIN_BOOL:
      return v == "true";
    case PLAIN_FLOAT:
      return v.toFloat();
    case PLAIN_DOUBLE:
      return v.toDouble();
    case QSTRING_LIST:
      return v.split('\n');
    default:
      return QVariant();
  }
}

KV::KV(const QString& name_, const QVariant& v_, const ValueChecker& checker_) : name(name_), v(v_), checker(checker_) {}

QString KV::valueToString() const {
  return checker.valueToString(v);
}

QString KV::valueToString(const QVariant& v_) const {
  return checker.valueToString(v_);
}
