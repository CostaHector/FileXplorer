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
inline bool CompatibleJsonKey(QVariantHash& vh) {
  auto itPS = vh.find("ProductionStudio");
  if (itPS != vh.cend()) {
    vh[JSONKey::Studio] = itPS.value();
    vh.erase(itPS);
    return true;
  }
  return false;
}

bool ClearPerformerAndStudio(QVariantHash& dict);
bool InsertPerfsPairToDictByNameHint(QVariantHash& dict);
bool AppendPerfsToDict(QVariantHash& dict, const QStringList& performerList);
bool InsertStudioPairIntoDict(QVariantHash& dict);
bool UpdateStudio(QVariantHash& dict, const QString& productionStudio);
}  // namespace VariantHashHelper

namespace JsonFileHelper {
QVariantHash GetMovieFileJsonDict(const QString& fileAbsPath, const QString& performersListStr = "", const QString& productionStudio = "");
QVariantHash GetDefaultJsonFile(const QString& fileName = "", const qint64& fileSz = 0);

bool MovieJsonDumper(const QVariantHash& dict, const QString& movieJsonItemPath);

QVariantHash JsonStr2Dict(const QString& jsonStr);
QVariantHash MovieJsonLoader(const QString& movieJsonItemPath);

QString GetJsonFilePath(const QString& vidsPath);

int ConstructJsonForVids(const QString& path, const QString& productionStudio = "", const QString& performersListStr = "");

int JsonPerformersKeyValuePairAdd(const QString& path);

int JsonProductionStudiosKeyValuePairAdd(const QString& path);

int JsonValuePerformersProductionStudiosCleaner(const QString& path);

int JsonValuePerformersAdder(const QString& path, const QString& performers);

int JsonValueProductionStudioSetter(const QString& path, const QString& _productionStudio);
}  // namespace JsonFileHelper

#endif  // JSONFILEHELPER_H
