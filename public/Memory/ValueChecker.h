#ifndef VALUECHECKER_H
#define VALUECHECKER_H
#include <QStringList>
#include <QVariant>
#include <QSet>

namespace VALUE_CHECKER_TYPE {
enum VALUE_TYPE {
  ERROR_TYPE = 0,
  PLAIN_STR = 1,
  PLAIN_BOOL = 2,
  PLAIN_INT = 3,
  PLAIN_FLOAT = 4,
  PLAIN_DOUBLE,
  FILE_PATH,
  EXT_SPECIFIED_FILE_PATH,
  FOLDER_PATH,
  RANGE_INT,
  SWITCH_STRING,  // "010101"
  CANDIDATE_STRING,
  QSTRING_LIST,
  QBYTEARRAY
};
}

class ValueChecker {
 public:
  friend struct KV;

  explicit ValueChecker(const QStringList& candidates,  //
                        const VALUE_CHECKER_TYPE::VALUE_TYPE valueType_ = VALUE_CHECKER_TYPE::VALUE_TYPE::EXT_SPECIFIED_FILE_PATH);

  explicit ValueChecker(int minV_ = INT32_MIN, int maxV_ = INT32_MAX);
  explicit ValueChecker(const QSet<QChar>& chars = {'0', '1'}, int minLength = 1);

  explicit ValueChecker(const VALUE_CHECKER_TYPE::VALUE_TYPE valueType_);

  static bool isFileExist(const QString& path);
  static bool isFolderExist(const QString& path);
  bool isStrInCandidate(const QString& str) const;
  bool isSpecifiedExtensionFileExist(const QString& path) const;
  bool isIntInRange(const int v) const;
  bool isSwitchString(const QString& switchs) const;

  bool operator()(const QVariant& v) const;
  QString valueToString(const QVariant& v) const;
  QVariant strToQVariant(const QString& v) const;

 private:
  ValueChecker() = delete;
  VALUE_CHECKER_TYPE::VALUE_TYPE valueType;

  QSet<QString> m_strCandidates;  // e.g. extension candidates
  QSet<QChar> m_switchStates;
  int m_switchMinCnt;

  int minV;
  int maxV;
};

#endif // VALUECHECKER_H
