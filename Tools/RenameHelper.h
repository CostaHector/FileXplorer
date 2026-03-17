#ifndef RENAMEHELPER_H
#define RENAMEHELPER_H

#include <QStringList>
namespace RenameHelper {

QStringList ReplaceRename(const QStringList& replaceeList, const QString& oldString, const QString& newString, bool regexEnable);
QStringList NumerizeRename(const QStringList& replaceeList, const QStringList& suffixs, const QString& baseName, const int startInd, const QString& namePattern, bool bUniqueExtCounter = true);
QStringList InsertRename(const QStringList& replaceeList, const QString& insertString, const int insertAt);
QStringList PrependParentFolderNameToFileName(const QStringList& parentFolders, const QStringList& completeNames, const QStringList& suffixs);

int GetDigitsCount(int number);
QString GetNameWithPatternIndex(const QString& baseName, const QString& namePattern, int index, int fieldWidth);
}

#endif  // RENAMEHELPER_H
