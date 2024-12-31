#include "PublicVariable.h"
#include "Tools/PathTool.h"

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

ValueChecker::ValueChecker(const QStringList& candidates, const VALUE_TYPE valueType_)
    : valueType{valueType_}, m_strCandidates{candidates.cbegin(), candidates.cend()} {}

ValueChecker::ValueChecker(int minV_, int maxV_) : valueType{RANGE_INT}, minV{minV_}, maxV{maxV_} {}

ValueChecker::ValueChecker(const QSet<QChar>& chars, int minLength) : valueType{SWITCH_STRING}, m_switchStates{chars}, m_switchMinCnt{minLength} {}

ValueChecker::ValueChecker(const VALUE_TYPE valueType_) : valueType(valueType_) {}

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
  const QString& ext = PATHTOOL::GetFileExtension(path);
  return isFileExist(path) && !ext.isEmpty() && isStrInCandidate(ext);
}

bool ValueChecker::isIntInRange(const int v) const {
  return minV <= v && v < maxV;
}

bool ValueChecker::isSwitchString(const QString& switchs) const {
  return switchs.size() >= m_switchMinCnt && QSet<QChar>(switchs.cbegin(), switchs.cend()) == m_switchStates;
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
