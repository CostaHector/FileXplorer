#ifndef MD5CALCULATOR_H
#define MD5CALCULATOR_H

#include <QStringList>
#include <QMap>
#include <QCryptographicHash>
#include "BytesRangeTool.h"

namespace MD5Calculator {
// <=0: full size bytes, >0, say 1024, only first 1024kB
QByteArray GetFileMD5(const QString& filepath, const BytesRangeTool::BytesRangeE firstBytesRangeInt = BytesRangeTool::BytesRangeE::ENTIRE_FILE, QCryptographicHash::Algorithm alg = QCryptographicHash::Algorithm::Md5);
QByteArray GetByteArrayMD5(const QByteArray& ba, QCryptographicHash::Algorithm alg = QCryptographicHash::Algorithm::Md5);
QList<QByteArray> GetBatchFileMD5(const QStringList& filepaths, const BytesRangeTool::BytesRangeE firstBytesRangeInt = BytesRangeTool::BytesRangeE::ENTIRE_FILE, QCryptographicHash::Algorithm alg = QCryptographicHash::Algorithm::Md5);
QString DisplayFilesMD5(const QStringList& fileAbsPaths);
QString MD5PrepathName2Table(const QList<QByteArray>& md5s, const QStringList& fileAbsPaths);
QString MD5DetailHtmlTable(const QList<QByteArray>& md5s, const QStringList& fileNames, const QStringList& fileDirs);
QString GetHashPlatformDependent(const QString& absFilePath);
}  // namespace MD5Calculator

#endif  // MD5CALCULATOR_H
