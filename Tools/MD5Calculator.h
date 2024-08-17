#ifndef MD5CALCULATOR_H
#define MD5CALCULATOR_H

#include <QStringList>
#include <QMap>

namespace MD5Calculator {
// <=0: full size bytes, >0, say 1024, only first 1024kB
QString GetMD5(const QString& filepath, const int onlyFirstByte = -1);
QStringList GetBatchMD5(const QStringList& filepaths, const int onlyFirstByte = -1);
QString DisplayFilesMD5(const QStringList& fileAbsPaths);
QString MD5PrepathName2Table(const QStringList& md5s, const QStringList& fileAbsPaths);
QString MD5DetailHtmlTable(const QStringList& md5s, const QStringList& fileNames, const QStringList& fileDirs);
}  // namespace MD5Calculator

#endif  // MD5CALCULATOR_H
