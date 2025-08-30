#ifndef CREATEFILEFOLDERHELPER_H
#define CREATEFILEFOLDERHELPER_H

#include <QStringList>

namespace CreateFileFolderHelper {
bool NewPlainTextFile(const QString& createIn, QString* newTextFileAbsPath = nullptr);
int NewJsonFile(const QString& createIn, const QStringList& basedOnFileNames);
bool NewFolder(const QString& createIn, QString* folderAbsPath = nullptr);
bool NewItems(const QString& createIn, const QString& namePattern="Page %03d.txt", int numStartIndex=1, int numEndIndex=11, bool isFolder = false);
}

#endif // CREATEFILEFOLDERHELPER_H
