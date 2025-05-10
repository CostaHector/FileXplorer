#ifndef JSONFILEHELPER_H
#define JSONFILEHELPER_H

#include <QVariantHash>
#include <QString>

namespace JSON_KEY {

#define JSON_KEY_MAPPING                                                         \
  JSON_KEY_ITEM(Name, 0, QString{})                                              \
  JSON_KEY_ITEM(Cast, 1, QStringList{})                                          \
  JSON_KEY_ITEM(Performers, 1, QStringList{}) /* deprecated, Use Cast instead */ \
  JSON_KEY_ITEM(Studio, 2, QString{})                                            \
  JSON_KEY_ITEM(Uploaded, 3, QString{})                                          \
  JSON_KEY_ITEM(Tags, 4, QStringList{})                                          \
  JSON_KEY_ITEM(Rate, 5, 0)                                                      \
  JSON_KEY_ITEM(Size, 6, 0)                                                      \
  JSON_KEY_ITEM(Resolution, 7, QString{})                                        \
  JSON_KEY_ITEM(Bitrate, 8, QString{})                                           \
  JSON_KEY_ITEM(Hot, 9, QVariantList{})                                          \
  JSON_KEY_ITEM(Detail, 10, QString{})                                           \
  JSON_KEY_ITEM(Duration, 11, 0)

// Key
enum JSON_KEY_E {
#define JSON_KEY_ITEM(enu, val, def) enu = val,
  JSON_KEY_MAPPING
#undef JSON_KEY_ITEM
  BUTT,
};

// Default Value: variable like JSON_DEF_VAL_Name
#define JSON_KEY_ITEM(enu, val, def) static const auto JSON_DEF_VAL_##enu = def;
JSON_KEY_MAPPING
#undef JSON_KEY_ITEM
}  // namespace JSON_KEY

namespace DictEditOperator {
struct CompatibleJsonKey {
  bool operator()(QVariantHash& dict) const;
};

struct ClearPerformerAndStudio {
  bool operator()(QVariantHash& dict) const;
};

struct ConstructStudioCastByName {
  bool operator()(QVariantHash& dict) const;
};

struct AppendPerfsToDict {
  AppendPerfsToDict(const QString& perfsStr);
  bool operator()(QVariantHash& dict) const;

private:
  const QStringList performerList;
};

struct UpdateStudio {
  UpdateStudio(const QString& _studio) : m_studio{_studio.trimmed()} {}
  bool operator()(QVariantHash& dict) const;

private:
  const QString m_studio;
};

struct StandardlizeJsonKey {
  bool operator()(QVariantHash& dict) const;
};

typedef std::function<bool(QVariantHash& dict)> JSON_DICT_PROCESS_T;
}  // namespace DictEditOperator

namespace JsonFileHelper {
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

QVariantHash GetJsonDictDefault(const QString& vidName = "", const qint64& fileSz = 0);
QVariantHash MovieJsonLoader(const QString& jsonFilePth);
QVariantHash DeserializedJsonStr2Dict(const QString& serializedJsonStr);
RET_ENUM InsertOrUpdateDurationStudioCastTags(const QString& jsonPth, int duration, const QString& studio, const QString& cast, const QString& tags);
bool DumpJsonDict(const QVariantHash& dict, const QString& jsonFilePth);
QMap<uint, JsonDict2Table> ReadStudioCastTagsOut(const QString& path);

int SyncJsonNameValue(const QString& path);
int JsonFileKeyValueProcess(const QString& path, const DictEditOperator::JSON_DICT_PROCESS_T jDProc);
int JsonSyncKeyValueAccordingJsonFileName(const QString& path);

}  // namespace JsonFileHelper

#endif  // JSONFILEHELPER_H
