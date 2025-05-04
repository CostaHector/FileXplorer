#ifndef JSONFILEHELPER_H
#define JSONFILEHELPER_H

#include <QVariantHash>
#include <QString>

namespace JSONKey {
const QString Name = "Name";
const QString Performers = "Performers";
const QString Studio = "Studio";
const QString Uploaded = "Uploaded";
const QString Tags = "Tags";
const QString Rate = "Rate";
const QString Size = "Size";
const QString Resolution = "Resolution";
const QString Bitrate = "Bitrate";
const QString Hot = "Hot";
const QString Detail = "Detail";
const QString Duration = "Duration";
const QStringList JsonKeyListOrder{Name, Performers, Studio, Uploaded, Tags, Rate, Size, Resolution, Bitrate, Hot, Detail};
}  // namespace JSONKey

namespace VariantHashHelper {
struct CompatibleJsonKey {
  bool operator()(QVariantHash& dict) const;
};

struct ClearPerformerAndStudio {
  bool operator()(QVariantHash& dict) const;
};

struct InsertPerfsPairToDictByNameHint {
  bool operator()(QVariantHash& dict) const;
};

struct AppendPerfsToDict {
  AppendPerfsToDict(const QString& perfsStr);
  bool operator()(QVariantHash& dict) const;

 private:
  const QStringList performerList;
};

struct InsertStudioPairIntoDict {
  bool operator()(QVariantHash& dict) const;
};

struct UpdateStudio {
  UpdateStudio(const QString& _studio) : studio{_studio.trimmed()} {};
  bool operator()(QVariantHash& dict) const;

 private:
  const QString studio;
};

typedef std::function<bool(QVariantHash& dict)> JSON_DICT_PROCESS_T;
}  // namespace VariantHashHelper

namespace JsonFileHelper {
enum RET_ENUM {
  CHANGED_WRITE_FILE_FAILED = -1000,
  OK = 0,
  CHANGED_OK,
};

struct JsonDict2Table {
  QString Studio;
  QString Cast;
  QString Tags;
};

uint CalcFileHash(const QString& vidPth);

QVariantHash GetJsonDictByMovieFile(const QString& vidFilePth, const QString& castStr = "", const QString& studio = "");
QVariantHash GetJsonDictDefault(const QString& vidName = "", const qint64& fileSz = 0);
QVariantHash MovieJsonLoader(const QString& jsonFilePth);
QVariantHash DeserializedJsonStr2Dict(const QString& serializedJsonStr);
RET_ENUM InsertOrUpdateDurationStudioCastTags(const QString& jsonPth, int duration, const QString& studio, const QString& cast, const QString& tags);
bool DumpJsonDict(const QVariantHash& dict, const QString& jsonFilePth);
QMap<uint, JsonDict2Table> ReadStudioCastTagsOut(const QString& path);


int ConstructJsonFileForVideosUnderPath(const QString& path, const QString& productionStudio = "", const QString& performersListStr = "");
int JsonFileKeyValueProcess(const QString& path, const VariantHashHelper::JSON_DICT_PROCESS_T jDProc);
}  // namespace JsonFileHelper

#endif  // JSONFILEHELPER_H
