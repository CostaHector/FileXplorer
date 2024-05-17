#ifndef MD5CALCULATOR_H
#define MD5CALCULATOR_H

#include <QStringList>
#include <QMap>

namespace MD5Calculator {

QString getMd5(const QString& filepath);
QStringList getBatchMD5(const QStringList& filepaths);
QString DisplayFilesMD5(const QStringList& fileAbsPaths);
QString MD5PrepathName2Table(const QStringList& md5s, const QStringList& fileAbsPaths);
QString MD5DetailHtmlTable(const QStringList& md5s, const QStringList& fileNames, const QStringList& fileDirs);
}  // namespace MD5Calculator

#endif  // MD5CALCULATOR_H
