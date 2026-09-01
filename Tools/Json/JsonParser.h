#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QString>
#include <QStringList>
#include "JsonOp.h"

namespace JsonParser {
JsonOp::ResultE ValidateSampleMd5AndVidName(const int localSampleMD5Size, const int localVidNameSize);

// jsonAbsPath, &sampleMd5, &name, &vidName, &size, &duration, &studio, &casts, &tags, &rate, &detail
JsonOp::ResultE ParseEssentialFieldJson(const QString& jsonFilePath,
                             QByteArray* pSampleMd5Val = nullptr, QString* pName = nullptr, QString* pVidName = nullptr,
                             qint64* pSize = nullptr, int* pDuration = nullptr,
                             QString* pStudio = nullptr, QStringList* pCasts = nullptr, QStringList* pTags = nullptr,
                             int* pRate = nullptr, QString* pDetail = nullptr);

int GetDurationFromJsonFile(const QString& jsonFullPath, bool* bSucceed, int defaultDurationValue);
int GetRateFromJsonFile(const QString& jsonFullPath, int defaultRateValue=0);
qint64 GetSizeFromJsonFile(const QString& jsonFullPath, qint64 defaultSizeValue=0);
QByteArray GetMD5FromJsonFile(const QString& jsonFullPath);
QStringList GetTagsFromJsonFile(const QString& jsonFullPath);
}

#endif // JSONPARSER_H
