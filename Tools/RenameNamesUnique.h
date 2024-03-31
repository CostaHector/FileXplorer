#ifndef RENAMENAMESUNIQUE_H
#define RENAMENAMESUNIQUE_H

#include <iterator>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <QSet>
#include <QString>

#include "FileOperation/FileOperation.h"
using std::ostringstream;

class RenameNamesUnique {
 public:
  RenameNamesUnique(const QString& pre,
                    const QStringList& relNameList,
                    const QStringList& oldCompleteNameList,
                    const QStringList& oldSuffixList,
                    const QStringList& newCompleteNameList,
                    const QStringList& newSuffixList,
                    const bool isIncludeSubDir = true);
  RenameNamesUnique(const RenameNamesUnique&) = delete;
  RenameNamesUnique& operator=(const RenameNamesUnique& rhs) = delete;

  static QSet<QString> getOccupiedPostPath(const QString& pre, const QStringList& leftRoots, const QStringList& leftNames, bool includeSub = true);
  static QString join2Path(const QString& rel2Name, const QString& name);
  static QStringList join2Path(const QString& rel2Name, const QStringList& names);
  static bool CheckConflict(QSet<QString> occupied,
                            const QStringList& leftRoots,
                            const QStringList& leftNames,
                            const QStringList& rightNames,
                            QStringList& conflictNames);

  operator bool() const { return isInputValid() and m_conflictNames.empty(); }
  bool operator()();

  QString Details() const {
    return QString(
               "arr len unequal:[%1]; "
               "name line empty[%2]; ")
               .arg(m_isArrLenUnequal)
               .arg(m_nameLineEmpty) +
           "Conflict list: [" + m_conflictNames.join('|') + "].";
  }

  FileOperation::BATCH_COMMAND_LIST_TYPE getRenameCommands() const;

 private:
  inline bool isInputValid() const { return not m_isArrLenUnequal and not m_nameLineEmpty; }

  bool m_isArrLenUnequal = false;
  bool m_nameLineEmpty = false;

  const QString& m_pre;
  const QStringList& m_relNameList;
  const QStringList& m_oldCompleteNameList;
  const QStringList& m_oldSuffixList;
  const QStringList& m_newCompleteNameList;
  const QStringList& m_newSuffixList;
  const bool m_isIncludeSubDir;

  QStringList m_leftNames, m_rightNames;

  QStringList m_conflictNames;
  QSet<QString> m_occupiedNames;
};

#endif  // RENAMENAMESUNIQUE_H
