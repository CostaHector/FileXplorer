#include "JsonFileHelper.h"
#include "Tools/NameTool.h"
#include "public/PathTool.h"
#include "Tools/PerformersManager.h"
#include "Tools/ProductionStudioManager.h"
#include "public/DisplayEnhancement.h"
#include "public/PublicVariable.h"
#include "public/PublicTool.h"

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
    dict[JSON_KEY::StudioS] = itPS.value();
    dict.erase(itPS);
    return true;
  }
  return false;
}

bool ClearPerformerAndStudio::operator()(QVariantHash& dict) const {
  bool cleared = false;
  auto perfIt = dict.find(JSON_KEY::PerformersS);
  if (perfIt != dict.cend() && !perfIt.value().toStringList().isEmpty()) {
    perfIt.value().clear();
    cleared = true;
  }
  auto studioIt = dict.find(JSON_KEY::StudioS);
  if (studioIt != dict.cend() && !studioIt.value().toString().isEmpty()) {
    studioIt.value().clear();
    cleared = true;
  }
  return cleared;
}

bool InsertPerfsPairToDictByNameHint::operator()(QVariantHash& dict) const {
  auto perfIt = dict.find(JSON_KEY::PerformersS);
  if (perfIt != dict.cend() && !perfIt.value().toStringList().isEmpty()) {
    return false;
  }
  const QString& sentence = dict.value(JSON_KEY::NameS, "").toString();
  static const PerformersManager& pm = PerformersManager::getIns();
  dict.insert(JSON_KEY::PerformersS, pm(sentence));
  return true;
}

AppendPerfsToDict::AppendPerfsToDict(const QString& perfsStr) : performerList{NameTool()(perfsStr)} {}

bool AppendPerfsToDict::operator()(QVariantHash& dict) const {
  auto perfIt = dict.find(JSON_KEY::PerformersS);
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
    dict[JSON_KEY::PerformersS] = afterLst;
  }
  return true;
}

bool InsertStudioPairIntoDict::operator()(QVariantHash& dict) const {
  auto studioIt = dict.find(JSON_KEY::StudioS);
  if (studioIt != dict.cend() && !studioIt.value().toString().isEmpty()) {
    return false;
  }
  const QString& sentence = dict.value(JSON_KEY::NameS, "").toString();
  static const ProductionStudioManager& psm = ProductionStudioManager::getIns();
  dict.insert(JSON_KEY::StudioS, psm.hintStdStudioName(sentence));
  return true;
}
bool UpdateStudio::operator()(QVariantHash& dict) const {
  auto studioIt = dict.find(JSON_KEY::StudioS);
  if (studioIt == dict.cend()) {
    dict[JSON_KEY::StudioS] = studio;
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
QVariantHash GetJsonDictByMovieFile(const QString& vidFilePth, const QString& castStr, const QString& studio) {
  const QStringList& performersList = NameTool()(castStr);
  const QFileInfo fi{vidFilePth};
  return QVariantHash{
      {JSON_KEY::NameS, fi.baseName()},                                          //
      {JSON_KEY::PerformersS, performersList},                                   //
      {JSON_KEY::StudioS, studio},                                               //
      {JSON_KEY::UploadedS, fi.birthTime().toString("yyyyMMdd")},                //
      {JSON_KEY::TagsS, QStringList()},                                          //
      {JSON_KEY::RateS, -1},                                                     //
      {JSON_KEY::SizeS, FILE_PROPERTY_DSP::sizeToHumanReadFriendly(fi.size())},  //
      {JSON_KEY::ResolutionS, ""},                                               //
      {JSON_KEY::BitrateS, ""},                                                  //
      {JSON_KEY::HotS, QVariantList{}},                                          //
      {JSON_KEY::DetailS, ""},                                                   //
      {JSON_KEY::DurationS, 0}                                                   //
  };
}

QVariantHash GetJsonDictDefault(const QString& vidName, const qint64& fileSz) {
  return QVariantHash{
      {JSON_KEY::NameS, vidName},                                             //
      {JSON_KEY::PerformersS, QStringList{}},                                 //
      {JSON_KEY::StudioS, ""},                                                //
      {JSON_KEY::UploadedS, ""},                                              //
      {JSON_KEY::TagsS, QStringList{}},                                       //
      {JSON_KEY::RateS, -1},                                                  //
      {JSON_KEY::SizeS, FILE_PROPERTY_DSP::sizeToHumanReadFriendly(fileSz)},  //
      {JSON_KEY::ResolutionS, ""},                                            //
      {JSON_KEY::BitrateS, ""},                                               //
      {JSON_KEY::HotS, QList<QVariant>{}},                                    //
      {JSON_KEY::DetailS, ""},                                                //
      {JSON_KEY::DurationS, 0}                                                //
  };
}

bool DumpJsonDict(const QVariantHash& dict, const QString& jsonFilePth) {
  auto jsonObject = QJsonObject::fromVariantHash(dict);
  QJsonDocument document;
  document.setObject(jsonObject);
  const auto& byteArray = document.toJson(QJsonDocument::JsonFormat::Indented);
  QFile jsonFile{jsonFilePth};
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

int ConstructJsonFileForVideosUnderPath(const QString& path, const QString& productionStudio, const QString& performersListStr) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] not a directory", qPrintable(path));
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;
  QDirIterator it{path, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QString& vidPath = it.filePath();
    const QString jsonPath{PATHTOOL::FileExtReplacedWithJson(vidPath)};
    if (QFile::exists(jsonPath)) {
      qDebug("File jsonPath[%s] is not exist", qPrintable(jsonPath));
      continue;
    }
    const auto& dict = GetJsonDictByMovieFile(vidPath, performersListStr, productionStudio);
    succeedCnt += DumpJsonDict(dict, jsonPath);
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
    succeedCnt += DumpJsonDict(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json processed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}

QVariantHash MovieJsonLoader(const QString& jsonFilePth) {
  const QString& json_string = TextReader(jsonFilePth);
  return DeserializedJsonStr2Dict(json_string);
}

QVariantHash DeserializedJsonStr2Dict(const QString& serializedJsonStr) { if (serializedJsonStr.isEmpty()) {
    return {};
  }
  QJsonParseError jsonErr;
  QJsonDocument json_doc = QJsonDocument::fromJson(serializedJsonStr.toUtf8(), &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError) {
    qWarning("Error parse json string %d char(s): %s", serializedJsonStr.size(), qPrintable(jsonErr.errorString()));
    return {};
  }
  const QJsonObject& rootObj = json_doc.object();
  QVariantHash dict = rootObj.toVariantHash();
  CompatibleJsonKey()(dict);
  return dict;
}

RET_ENUM InsertOrUpdateDurationStudioCastTags(const QString& jsonPth, int duration, const QString& studio, const QString& cast, const QString& tags) {
  QVariantHash dict;
  bool changed{false};
  if (QFile::exists(jsonPth)) {
    dict = MovieJsonLoader(jsonPth);
  } else {
    const QString& name = PATHTOOL::GetBaseName(jsonPth);
    dict = GetJsonDictDefault(name);
    changed = true;
  }

  QHash<QString, QVariant>::iterator it;
  if (duration != 0) {
    it = dict.find(JSON_KEY::SizeS);  // here size is the duration
    if (it != dict.cend() && it.value().toInt() != duration) {
      it->setValue(duration);
      changed = true;
    }
  }
  if (!studio.isEmpty()) {
    it = dict.find(JSON_KEY::StudioS);
    if (it != dict.cend() && it.value().toString() != studio) {
      it->setValue(studio);
      changed = true;
    }
  }
  if (!cast.isEmpty()) {
    it = dict.find(JSON_KEY::PerformersS);  // here cast is the Performers
    if (it != dict.cend() && it.value().toString() != cast) {
      it->setValue(cast);
      changed = true;
    }
  }
  if (!tags.isEmpty()) {
    it = dict.find(JSON_KEY::TagsS);
    if (it != dict.cend() && it.value().toString() != tags) {
      it->setValue(tags);
      changed = true;
    }
  }

  if (changed) {
    bool dumpResult = DumpJsonDict(dict, jsonPth);
    if (!dumpResult) {
      return CHANGED_WRITE_FILE_FAILED;
    } else {
      return CHANGED_OK;
    }
  }

  return OK;
}

QMap<uint, JsonDict2Table> ReadStudioCastTagsOut(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a dir", qPrintable(path));
    return {};
  }

  QMap<uint, JsonDict2Table> fileNameHash2Json;
  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = MovieJsonLoader(jsonPath);
    const QString& studio = dict.value(JSON_KEY::StudioS, "").toString();
    if (studio.isEmpty()) {
      continue;
    }
    const QString& cast = dict.value(JSON_KEY::PerformersS, "").toString();
    if (cast.isEmpty()) {
      continue;
    }
    const QString& tags = dict.value(JSON_KEY::TagsS, "").toString();
    if (tags.isEmpty()) {
      continue;
    }
    fileNameHash2Json[CalcFileHash(jsonPath)] = JsonDict2Table{studio, cast, tags};
  }
  qDebug("%d file contains Studio&Cast&Tags", fileNameHash2Json.size());
  return fileNameHash2Json;
}

uint CalcFileHash(const QString& vidPth) {
  return qHash(PATHTOOL::GetFileNameExtRemoved(vidPth));
}

}  // namespace JsonFileHelper
