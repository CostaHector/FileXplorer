#include "JsonFileHelper.h"
#include "Tools/NameTool.h"
#include "Tools/PathTool.h"
#include "Tools/PerformersManager.h"
#include "Tools/ProductionStudioManager.h"
#include "public/DisplayEnhancement.h"
#include "PublicVariable.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDirIterator>
#include <QFile>

namespace VariantHashHelper {
bool CompatibleJsonKey::operator()(QVariantHash& dict) const {
  auto itPS = dict.find("ProductionStudio");
  if (itPS != dict.cend()) {
    dict[JSONKey::Studio] = itPS.value();
    dict.erase(itPS);
    return true;
  }
  return false;
}

bool ClearPerformerAndStudio::operator()(QVariantHash& dict) const {
  bool cleared = false;
  auto perfIt = dict.find(JSONKey::Performers);
  if (perfIt != dict.cend() && !perfIt.value().toStringList().isEmpty()) {
    perfIt.value().clear();
    cleared = true;
  }
  auto studioIt = dict.find(JSONKey::Studio);
  if (studioIt != dict.cend() && !studioIt.value().toString().isEmpty()) {
    studioIt.value().clear();
    cleared = true;
  }
  return cleared;
}

bool InsertPerfsPairToDictByNameHint::operator()(QVariantHash& dict) const {
  auto perfIt = dict.find(JSONKey::Performers);
  if (perfIt != dict.cend() && !perfIt.value().toStringList().isEmpty()) {
    return false;
  }
  const QString& sentence = dict.value(JSONKey::Name, "").toString();
  static const PerformersManager& pm = PerformersManager::getIns();
  dict.insert(JSONKey::Performers, pm(sentence));
  return true;
}

AppendPerfsToDict::AppendPerfsToDict(const QString& perfsStr) : performerList{NameTool()(perfsStr)} {}

bool AppendPerfsToDict::operator()(QVariantHash& dict) const {
  auto perfIt = dict.find(JSONKey::Performers);
  QStringList afterLst;
  afterLst += performerList;
  if (perfIt != dict.cend()) {
    afterLst += perfIt.value().toStringList();
    afterLst.sort();
    afterLst.removeDuplicates();
    if (afterLst == perfIt.value().toStringList()) {
      return false;
    }
    perfIt.value() = afterLst;
  } else {
    afterLst.sort();
    afterLst.removeDuplicates();
    dict[JSONKey::Performers] = afterLst;
  }
  return true;
}

bool InsertStudioPairIntoDict::operator()(QVariantHash& dict) const {
  auto studioIt = dict.find(JSONKey::Studio);
  if (studioIt != dict.cend() && !studioIt.value().toString().isEmpty()) {
    return false;
  }
  const QString& sentence = dict.value(JSONKey::Name, "").toString();
  static const ProductionStudioManager& psm = ProductionStudioManager::getIns();
  dict.insert(JSONKey::Studio, psm.hintStdStudioName(sentence));
  return true;
}
bool UpdateStudio::operator()(QVariantHash& dict) const {
  auto studioIt = dict.find(JSONKey::Studio);
  if (studioIt == dict.cend()) {
    dict[JSONKey::Studio] = studio;
    return true;
  }
  if (studioIt.value().toString() == studio) {
    return false;
  }
  studioIt.value() = studio;
  return true;
}
}  // namespace VariantHashHelper

namespace JsonFileHelper {
using namespace VariantHashHelper;
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

int JsonFileKeyValueProcess(const QString& path, const VariantHashHelper::JSON_DICT_PROCESS_T jDProc) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a dir", qPrintable(path));
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (!jDProc(dict)) {
      continue;
    }
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json processed", succeedCnt, tryConstuctCnt);
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
  CompatibleJsonKey()(dict);
  return dict;
}
}  // namespace JsonFileHelper
