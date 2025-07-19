#ifndef RENAMEHELPER_H
#define RENAMEHELPER_H

#include <QStringList>
namespace RenameHelper {

QStringList ReplaceRename(const QStringList& replaceeList, const QString& oldString, const QString& newString, bool regexEnable);
QStringList NumerizeReplace(const QStringList& replaceeList, const QStringList& suffixs, const QString& baseName, const int startInd, const QString& namePattern);
QStringList InsertRename(const QStringList& replaceeList, const QString& insertString, const int insertAt);
QStringList PrependParentFolderNameToFileName(const QStringList& parentFolders, const QStringList& completeNames, const QStringList& suffixs);

}

#endif  // RENAMEHELPER_H
