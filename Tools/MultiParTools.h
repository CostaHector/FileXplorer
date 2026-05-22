#ifndef MULTIPARTOOLS_H
#define MULTIPARTOOLS_H

#include <QStringList>
namespace MultiParTools {
std::pair<bool, int> CreatePar2(const QStringList& filesAbsPath, int rateOfRedundancy=10);
std::pair<bool, int> VerifyPar2(const QStringList& filesAbsPath);
std::pair<bool, bool> isFileNeedRecovery(const QString& fileAbsPath, const QString& parFileAbsPath);
}

#endif // MULTIPARTOOLS_H
