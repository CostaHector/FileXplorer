#ifndef RENAMENAMESUNIQUE_H
#define RENAMENAMESUNIQUE_H

#include <QSet>
#include <QString>
#include "FileOperatorPub.h"

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
  static bool CheckConflict(QSet<QString> occupied, const QStringList& leftRoots, const QStringList& leftNames, const QStringList& rightNames, QStringList& conflictNames);

  explicit operator bool() const { return isInputValid() && m_conflictNames.empty(); }
  bool operator()();
  QString Details() const {                                                                            //
    return QString("bLenUnequal:[%1],bBasenameEmpty[%2]").arg(m_isArrLenUnequal).arg(m_nameLineEmpty)  //
           + "Conflicts list: [" + m_conflictNames.join('\n') + "].";
  }

  FileOperatorType::BATCH_COMMAND_LIST_TYPE getRenameCommands() const;

 private:
  inline bool isInputValid() const {  //
    return !m_isArrLenUnequal && !m_nameLineEmpty;
  }

  bool m_isArrLenUnequal = false;
  bool m_nameLineEmpty = false;

  const QString m_pre;
  const QStringList m_relNameList;
  const QStringList m_oldCompleteNameList;
  const QStringList m_oldSuffixList;
  const QStringList m_newCompleteNameList;
  const QStringList m_newSuffixList;
  const bool m_isIncludeSubDir;

  QStringList m_leftNames, m_rightNames;

  QStringList m_conflictNames;
  QSet<QString> m_occupiedNames;
};

#endif  // RENAMENAMESUNIQUE_H
