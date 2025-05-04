#ifndef JSONFILEHELPER_H
#define JSONFILEHELPER_H

#include <QVariantHash>
#include <QString>
#include "public/PublicMacro.h"
namespace JSON_KEY {
enum JSON_KEY_E {
  Name = 0,
  Cast = 1,
  Performers = Cast,
  Studio,
  Uploaded,
  Tags,
  Rate,
  Size,
  Resolution,
  Bitrate,
  Hot,
  Detail,
  Duration,
  BUTT,
};

const QString NameS = VOLUME_ENUM_TO_STRING(Name);
const QString PerformersS = VOLUME_ENUM_TO_STRING(Performers);
const QString StudioS = VOLUME_ENUM_TO_STRING(Studio);
const QString UploadedS = VOLUME_ENUM_TO_STRING(Uploaded);
const QString TagsS = VOLUME_ENUM_TO_STRING(Tags);
const QString RateS = VOLUME_ENUM_TO_STRING(Rate);
const QString SizeS = VOLUME_ENUM_TO_STRING(Size);
const QString ResolutionS = VOLUME_ENUM_TO_STRING(Resolution);
const QString BitrateS = VOLUME_ENUM_TO_STRING(Bitrate);
const QString HotS = VOLUME_ENUM_TO_STRING(Hot);
const QString DetailS = VOLUME_ENUM_TO_STRING(Detail);
const QString DurationS = VOLUME_ENUM_TO_STRING(Duration);
}  // namespace JSON_KEY

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
