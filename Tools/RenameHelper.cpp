#include "RenameHelper.h"
#include "Logger.h"
#include <QRegularExpression>

namespace RenameHelper {
QStringList ReplaceRename(const QStringList& replaceeList, const QString& oldString, const QString& newString, bool regexEnable) {
  if (replaceeList.isEmpty()) {
    return {};
  }
  if (oldString.isEmpty()) {
    return {};
  }
  if (oldString == newString) {
    return replaceeList;
  }
  QStringList replacedLst{replaceeList};
  if (!regexEnable) {
    for (QString& s : replacedLst) {
      s.replace(oldString, newString);
    }
    return replacedLst;
  }
  const QRegularExpression repRegex{oldString};
  if (!repRegex.isValid()) {
    LOG_W("regular expression error[%s]", qPrintable(oldString));
    return {};
  }
  for (QString& s : replacedLst) {
    s.replace(repRegex, newString);
  }
  return replacedLst;
}

QStringList NumerizeReplace(const QStringList& replaceeList, const QStringList& suffixs, const QString& baseName, const int startInd, const QString& namePattern, bool bUniqueExtCounter) {
  if (replaceeList.isEmpty()) {
    return {};
  }
  if (replaceeList.size() == 1) { // no need add number
    return {baseName};
  }

  if (!namePattern.contains("%1")) {
    LOG_W("namePattern[%s] must contain at least one '%%1'", qPrintable(namePattern));
    return {};
  }

  QStringList numerizedNames;
  numerizedNames.reserve(suffixs.size());
  if (!bUniqueExtCounter) { // 不区分后缀, 统一计数
    int fieldWidth{0};
    int val = startInd + suffixs.size() - 1;  // start 9, len 1, max index should be 9, not 10
    while (val != 0) {
      val /= 10;
      ++fieldWidth;
    }
    // 补充0到fieldWidth位
    for (int i = 0; i < suffixs.size(); ++i) {
      numerizedNames.append(baseName + namePattern.arg(i + startInd, fieldWidth, 10, QChar{'0'}));
    }
    return numerizedNames;
  } else { // 区分后缀, 独立计数
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
    for (const QString& suf : suffixs) {
      if (!sufCurIndex.contains(suf)) {  // no need to no. because this extension count <= 1
        numerizedNames.append(baseName);
        continue;
      }
      QString newBaseName = baseName;
      newBaseName += namePattern.arg(sufCurIndex[suf], suf2fieldWidth[suf], 10, QChar('0'));
      numerizedNames.append(newBaseName);
      sufCurIndex[suf] += 1;
    }
    return numerizedNames;
  }
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

QStringList PrependParentFolderNameToFileName(const QStringList& parentFolders, const QStringList& completeNames, const QStringList& suffixs) {
  const int N1 = parentFolders.size();
  const int N2 = completeNames.size();
  const int N3 = suffixs.size();
  if (N1 == 0) {
    return {};
  }
  if (N1 != N2 || N2 != N3) {
    return {};
  }
  QStringList ansNames;
  ansNames.reserve(N1);
  for (int i = 0; i < parentFolders.size(); ++i) {
    if (suffixs[i].isEmpty()) { // only works for files
      ansNames.push_back(completeNames[i]);
      continue;
    }
    QString prepath{parentFolders[i]};
    if (prepath.isEmpty()) {
      ansNames.append(completeNames[i]); // not parent folder
    } else {
      ansNames.push_back(prepath.replace('/', ' ') + ' ' + completeNames[i]);
    }
  }
  return ansNames;
}

}  // namespace RenameHelper

