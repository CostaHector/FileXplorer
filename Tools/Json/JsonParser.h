#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QString>
#include <QStringList>

namespace JsonParser {
// jsonAbsPath, &sampleMd5, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail
bool ParseEssentialFieldJson(const QString& jsonFilePath,
                             QByteArray* pSampleMd5Val = nullptr, QString* pName = nullptr, QString* pVidName = nullptr,
                             qint64* pSize = nullptr, int* pDuration = nullptr,
                             QString* pStudio = nullptr, QStringList* pCasts = nullptr, QStringList* pTags = nullptr,
                             int* pRate = nullptr, QString* pDetail = nullptr);

int GetDurationFromJsonFile(const QString& jsonFullPath, bool* bSucceed, int defaultDurationValue=0);
int GetRateFromJsonFile(const QString& jsonFullPath, int defaultRateValue=0);
QByteArray GetMD5FromJsonFile(const QString& jsonFullPath);
}

#endif // JSONPARSER_H
