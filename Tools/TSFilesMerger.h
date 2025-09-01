#ifndef TSFILESMERGER_H
#define TSFILESMERGER_H

#include <QStringList>

namespace TSFilesMerger {
bool checkTsFilesConsistent(const QStringList& tsAbsPathList);
std::pair<bool, QString> mergeTsFiles(const QString& filesIn, const QStringList& tsNames);
}

#endif // TSFILESMERGER_H
