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
QVariantHash GetMovieFileJsonDict(const QString& fileAbsPath, const QString& performersListStr = "", const QString& productionStudio = "");
QVariantHash GetDefaultJsonFile(const QString& fileName = "", const qint64& fileSz = 0);
QVariantHash JsonStr2Dict(const QString& jsonStr);
QVariantHash MovieJsonLoader(const QString& movieJsonItemPath);

bool MovieJsonDumper(const QVariantHash& dict, const QString& movieJsonItemPath);
QString GetJsonFilePath(const QString& vidsPath);

int ConstructJsonForVids(const QString& path, const QString& productionStudio = "", const QString& performersListStr = "");
int JsonFileKeyValueProcess(const QString& path, const VariantHashHelper::JSON_DICT_PROCESS_T jDProc);

}  // namespace JsonFileHelper

#endif  // JSONFILEHELPER_H
