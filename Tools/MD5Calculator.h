#ifndef MD5CALCULATOR_H
#define MD5CALCULATOR_H

#include <QStringList>
#include <QMap>
#include <QCryptographicHash>

namespace MD5Calculator {
constexpr int BYTE_ALL = -1;
constexpr int BYTE_1 = 1024;
constexpr int BYTE_2 = 2048;
constexpr int BYTE_4 = 4096;
constexpr int BYTE_8 = 8192;
constexpr int BYTE_1024 = 1024 * 1024;
// <=0: full size bytes, >0, say 1024, only first 1024kB
QString GetFileMD5(const QString& filepath, const int firstBytesRangeInt = BYTE_ALL, QCryptographicHash::Algorithm alg = QCryptographicHash::Algorithm::Md5);
QString GetByteArrayMD5(const QByteArray& ba, QCryptographicHash::Algorithm alg = QCryptographicHash::Algorithm::Md5);
QStringList GetBatchFileMD5(const QStringList& filepaths, const int firstBytesRangeInt = BYTE_ALL, QCryptographicHash::Algorithm alg = QCryptographicHash::Algorithm::Md5);
QString DisplayFilesMD5(const QStringList& fileAbsPaths);
QString MD5PrepathName2Table(const QStringList& md5s, const QStringList& fileAbsPaths);
QString MD5DetailHtmlTable(const QStringList& md5s, const QStringList& fileNames, const QStringList& fileDirs);
QString GetHashPlatformDependent(const QString& absFilePath);
}  // namespace MD5Calculator

#endif  // MD5CALCULATOR_H
