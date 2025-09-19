#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QVariantHash>
#include <QString>

namespace JsonHelper {
static constexpr char JSON_EXT[]{".json"};
static constexpr int JSON_EXT_LENGTH = sizeof(JsonHelper::JSON_EXT) / sizeof(JsonHelper::JSON_EXT[0]) - 1;
static constexpr char ELEMENT_JOINER = ',';
enum RET_ENUM {
  CHANGED_WRITE_FILE_FAILED = -1000,
  NOCHANGED_OK = 0,
  CHANGED_OK,
};

struct JsonDict2Table {
  QString Studio;
  QStringList Cast;
  QStringList Tags;
};

uint CalcFileHash(const QString& vidPth);

QVariantHash MovieJsonLoader(const QString& jsonFilePth);
QJsonObject GetJsonObject(const QString& jsonFilePath);
QVariantHash DeserializedJsonStr2Dict(const QString& serializedJsonStr);
RET_ENUM InsertOrUpdateDurationStudioCastTags(const QString& jsonPth, int duration, const QString& studio, const QString& cast, const QString& tags);
bool DumpJsonDict(const QVariantHash& dict, const QString& jsonFilePth);
QMap<uint, JsonDict2Table> ReadStudioCastTagsOut(const QString& path);
}  // namespace JsonHelper

#endif  // JSONHELPER_H
