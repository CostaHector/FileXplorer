#include "RenameHelper.h"
#include <QRegularExpression>

namespace RenameHelper {
QStringList ReplaceRename(const QStringList& replaceeList, const QString& oldString, const QString& newString, bool regexEnable) {
  if (replaceeList.isEmpty()) {
    return {};
  }
  if (oldString.isEmpty()) {
    return {};
  }
  if (!regexEnable) {
    QStringList replacedLst(replaceeList);
    for (QString& s : replacedLst) {
      s.replace(oldString, newString);
    }
    return replacedLst;
  }
  QRegularExpression repRegex(oldString);
  if (!repRegex.isValid()) {
    qWarning("regular expression error[%s]", qPrintable(oldString));
    return {};
  }
  QStringList replacedLst(replaceeList);
  for (QString& s : replacedLst) {
    s.replace(repRegex, newString);
  }
  return replacedLst;
}

QStringList NumerizeReplace(const QStringList& replaceeList, const QStringList& suffixs, const QString& baseName, const int startInd, const QString& namePattern) {
  if (replaceeList.isEmpty()) {
    return {};
  }
  if (!namePattern.contains("%1")) {
    qWarning("namePattern[%s] must contain at least one '%%1'", qPrintable(namePattern));
    return {};
  }
  QMap<QString, int> sufCntMap;
  for (const QString& suf : suffixs) {
    auto extIt = sufCntMap.find(suf);
    if (extIt != sufCntMap.end()) {
      ++extIt.value();
    } else {
      sufCntMap[suf] = 1;
    }
  }

  QMap<QString, int> sufCurIndex;  // each extension no. start
  QMap<QString, int> suf2fieldWidth;
  for (auto ext2Cnt = sufCntMap.cbegin(); ext2Cnt != sufCntMap.cend(); ++ext2Cnt) {
    if (ext2Cnt.value() <= 1) {
      continue;
    }
    sufCurIndex[ext2Cnt.key()] = startInd;
    int fieldWidth = 0;
    int val = startInd + ext2Cnt.value() - 1;  // start 9, len 1, max index should be 9, not 10
    while (val != 0) {
      val /= 10;
      ++fieldWidth;
    }
    suf2fieldWidth[ext2Cnt.key()] = fieldWidth;
  }

  QStringList numerizedNames;
  for (const QString& suf : suffixs) {
    if (not sufCurIndex.contains(suf)) {  // no need to no. because this extension count <= 1
      numerizedNames.append(baseName);
      continue;
    }
    QString newBaseName = baseName;
    if (!baseName.isEmpty()) {
      newBaseName += namePattern.arg(sufCurIndex[suf], suf2fieldWidth[suf], 10, QChar('0'));
    }
    numerizedNames.append(newBaseName);
    sufCurIndex[suf] += 1;
  }
  return numerizedNames;
}

QStringList InsertRename(const QStringList& replaceeList, const QString& insertString, const int insertAt) {
  if (replaceeList.isEmpty()) {
    return {};
  }
  if (insertString.isEmpty()) {
    return replaceeList;
  }
  QStringList afterInsert;
  for (const QString& replacee : replaceeList) {
    int realInsertAt = (insertAt > replacee.size()) ? replacee.size() : insertAt;
    afterInsert.append(replacee.left(realInsertAt) + insertString + replacee.mid(realInsertAt));
  }
  return afterInsert;
}

}  // namespace RenameHelper
