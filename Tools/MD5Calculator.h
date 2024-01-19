#ifndef MD5CALCULATOR_H
#define MD5CALCULATOR_H

#include <QStringList>
#include <QMap>

namespace MD5Calculator {

QString getMd5(const QString& filepath);
QStringList getBatchMD5(const QStringList& filepaths);
QString DisplayFilesMD5(const QStringList& md5s, const QStringList& fileNames, const QStringList& fileDirs);
QString DisplayFilesMD5(const QStringList& md5s, const QStringList& fileAbsPaths);
QString DisplayFilesMD5(const QStringList& fileAbsPaths);
}  // namespace MD5Calculator

#endif  // MD5CALCULATOR_H
