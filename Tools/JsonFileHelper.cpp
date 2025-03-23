#include "JsonFileHelper.h"
#include "Tools/NameTool.h"
#include "Tools/PathTool.h"
#include "Tools/PerformersManager.h"
#include "Tools/ProductionStudioManager.h"
#include "PublicVariable.h"
#include "public/DisplayEnhancement.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDirIterator>
#include <QFile>

namespace VariantHashHelper {
bool ClearPerformerAndStudio(QVariantHash& dict) {
  auto perfIt = dict.find(JSONKey::Performers);
  bool cleared = false;
  if (perfIt != dict.cend() && !perfIt.value().toStringList().isEmpty()) {
    perfIt.value().clear();
    cleared = true;
  }
  auto studioIt = dict.find(JSONKey::Studio);
  if (studioIt != dict.cend() && !perfIt.value().toString().isEmpty()) {
    studioIt.value().clear();
    cleared = true;
  }
  return cleared;
}

bool InsertPerfsPairToDictByNameHint(QVariantHash& dict) {
  auto perfIt = dict.find(JSONKey::Performers);
  if (perfIt != dict.cend() and !perfIt.value().toStringList().isEmpty()) {
    return false;
  }
  const QString& sentence = dict.value(JSONKey::Name, "").toString();
  static const PerformersManager& pm = PerformersManager::getIns();
  dict.insert(JSONKey::Performers, pm(sentence));
  return true;
}

bool AppendPerfsToDict(QVariantHash& dict, const QStringList& performerList) {
  auto perfIt = dict.find(JSONKey::Performers);
  if (perfIt != dict.cend()) {
    QStringList afterLst = perfIt.value().toStringList();
    afterLst += performerList;
    afterLst.removeDuplicates();
    if (afterLst == perfIt.value().toStringList()) {
      return false;
    }
    perfIt.value().toStringList().swap(afterLst);
  } else {
    dict[JSONKey::Performers] = performerList;
  }
  return true;
}

bool InsertStudioPairIntoDict(QVariantHash& dict) {
  auto studioIt = dict.find(JSONKey::Studio);
  if (studioIt != dict.cend() && !studioIt.value().toString().isEmpty()) {
    return false;
  }
  const QString& sentence = dict.value(JSONKey::Name, "").toString();
  static const ProductionStudioManager& psm = ProductionStudioManager::getIns();
  dict.insert(JSONKey::Studio, psm.hintStdStudioName(sentence));
  return true;
}
bool UpdateStudio(QVariantHash& dict, const QString& productionStudio) {
  auto studioIt = dict.find(JSONKey::Studio);
  if (studioIt == dict.cend()) {
    dict[JSONKey::Studio] = productionStudio;
    return true;
  }
  if (studioIt.value().toString() == productionStudio) {
    return false;
  }
  studioIt.value() = productionStudio;
  return true;
}
}  // namespace VariantHashHelper

namespace JsonFileHelper {
QVariantHash GetMovieFileJsonDict(const QString& fileAbsPath,        //
                                  const QString& performersListStr,  //
                                  const QString& productionStudio) {
  const QStringList& performersList = NameTool()(performersListStr);
  const QFileInfo fi{fileAbsPath};
  return QVariantHash{{JSONKey::Name, fi.baseName()},
                      {JSONKey::Performers, performersList},
                      {JSONKey::Studio, productionStudio},
                      {JSONKey::Uploaded, fi.birthTime().toString("yyyyMMdd")},
                      {JSONKey::Tags, QStringList()},
                      {JSONKey::Rate, -1},
                      {JSONKey::Size, FILE_PROPERTY_DSP::sizeToHumanReadFriendly(fi.size())},
                      {JSONKey::Resolution, ""},
                      {JSONKey::Bitrate, ""},
                      {JSONKey::Hot, QVariantList{}},
                      {JSONKey::Detail, ""}};
}

QVariantHash GetDefaultJsonFile(const QString& fileName, const qint64& fileSz) {
  return QVariantHash{{JSONKey::Name, fileName},                                            //
                      {JSONKey::Performers, QStringList{}},                                 //
                      {JSONKey::Studio, ""},                                                //
                      {JSONKey::Uploaded, ""},                                              //
                      {JSONKey::Tags, QStringList{}},                                       //
                      {JSONKey::Rate, -1},                                                  //
                      {JSONKey::Size, FILE_PROPERTY_DSP::sizeToHumanReadFriendly(fileSz)},  //
                      {JSONKey::Resolution, ""},                                            //
                      {JSONKey::Bitrate, ""},                                               //
                      {JSONKey::Hot, QList<QVariant>{}},                                    //
                      {JSONKey::Detail, ""}};
}

bool MovieJsonDumper(const QVariantHash& dict, const QString& movieJsonItemPath) {
  auto jsonObject = QJsonObject::fromVariantHash(dict);
  QJsonDocument document;
  document.setObject(jsonObject);
  const auto& byteArray = document.toJson(QJsonDocument::JsonFormat::Indented);
  QFile jsonFile{movieJsonItemPath};
  if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    jsonFile.close();
    return false;
  }
  QTextStream out(&jsonFile);
  out.setCodec("UTF-8");
  out << byteArray;
  jsonFile.close();
  return true;
}

QString GetJsonFilePath(const QString& vidsPath) {
  const QString& baseName = PATHTOOL::GetBaseName(vidsPath);
  const int lastSlashInd = vidsPath.lastIndexOf('/');
  return vidsPath.left(lastSlashInd + 1) + baseName + ".json";
}

int ConstructJsonForVids(const QString& path, const QString& productionStudio, const QString& performersListStr) {
  if (not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;
  QDirIterator it(path, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& vidPath = it.filePath();
    const QString& jsonPath = GetJsonFilePath(vidPath);
    if (QFile::exists(jsonPath)) {
      continue;
    }
    const auto& dict = GetMovieFileJsonDict(vidPath, performersListStr, productionStudio);
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json contructed succeed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}

int JsonPerformersKeyValuePairAdd(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryKVPairCnt = 0;

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (!VariantHashHelper::InsertPerfsPairToDictByNameHint(dict)) {
      continue;
    }
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryKVPairCnt;
  }
  if (tryKVPairCnt != succeedCnt) {
    qDebug("%d/%d json add performer key-value pair succeed", succeedCnt, tryKVPairCnt);
  }
  return succeedCnt;
}

int JsonProductionStudiosKeyValuePairAdd(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryKVPairCnt = 0;

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (!VariantHashHelper::InsertStudioPairIntoDict(dict)) {
      continue;
    }
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryKVPairCnt;
  }
  if (tryKVPairCnt != succeedCnt) {
    qDebug("%d/%d json add studio key-value pair succeed", succeedCnt, tryKVPairCnt);
  }
  return succeedCnt;
}

int JsonValuePerformersProductionStudiosCleaner(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryCleanCnt = 0;
  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (!VariantHashHelper::ClearPerformerAndStudio(dict)) {
      continue;
    }
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryCleanCnt;
  }
  if (tryCleanCnt != succeedCnt) {
    qDebug("%d/%d json clean performer or production studio name succeed", succeedCnt, tryCleanCnt);
  }
  return succeedCnt;
}

int JsonValuePerformersAdder(const QString& path, const QString& performers) {
  const QStringList& performerList = NameTool()(performers);
  if (performerList.isEmpty() || !QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (!VariantHashHelper::AppendPerfsToDict(dict, performerList)) {
      continue;
    }
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json add performer succeed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}

int JsonValueProductionStudioSetter(const QString& path, const QString& _productionStudio) {
  const QString& productionStudio = _productionStudio.trimmed();
  if (not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (!VariantHashHelper::UpdateStudio(dict, productionStudio)) {
      continue;
    }
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json add productionStudio set succeed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}

QVariantHash MovieJsonLoader(const QString& movieJsonItemPath) {
  QFile jsonFile{movieJsonItemPath};
  if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("[Normal] json file[%s] not found", qPrintable(movieJsonItemPath));
    return {};
  }
  QTextStream in(&jsonFile);
  in.setCodec("UTF-8");
  QString json_string = in.readAll();
  jsonFile.close();
  return JsonStr2Dict(json_string);
}

QVariantHash JsonStr2Dict(const QString& jsonStr) {
  QJsonParseError jsonErr;
  QJsonDocument json_doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError) {
    qWarning("Error parse json string %d char(s): %s", jsonStr.size(), qPrintable(jsonErr.errorString()));
    return {};
  }
  const QJsonObject& rootObj = json_doc.object();
  auto dict = rootObj.toVariantHash();
  VariantHashHelper::CompatibleJsonKey(dict);
  return dict;
}
}  // namespace JsonFileHelper
