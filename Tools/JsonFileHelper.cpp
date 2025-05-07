#include "JsonFileHelper.h"
#include "public/DisplayEnhancement.h"
#include "public/PathTool.h"
#include "public/PublicVariable.h"
#include "public/PublicTool.h"
#include "public/PublicMacro.h"
#include "Tools/NameTool.h"
#include "Tools/CastManager.h"
#include "Tools/StudiosManager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDirIterator>
#include <QFile>

namespace DictEditOperator {
bool CompatibleJsonKey::operator()(QVariantHash& dict) const {
  auto itPS = dict.find("ProductionStudio");
  if (itPS != dict.cend()) {
    dict[ENUM_TO_STRING(Studio)] = itPS.value();
    dict.erase(itPS);
    return true;
  }
  return false;
}

bool ClearPerformerAndStudio::operator()(QVariantHash& dict) const {
  bool cleared = false;
  auto perfIt = dict.find(ENUM_TO_STRING(Cast));
  if (perfIt != dict.cend() && !perfIt.value().toStringList().isEmpty()) {
    perfIt.value().clear();
    cleared = true;
  }
  auto studioIt = dict.find(ENUM_TO_STRING(Studio));
  if (studioIt != dict.cend() && !studioIt.value().toString().isEmpty()) {
    studioIt.value().clear();
    cleared = true;
  }
  return cleared;
}

bool ConstructStudioCastByName::operator()(QVariantHash& dict) const {
  auto nameIt = dict.find(ENUM_TO_STRING(Name));
  if (nameIt == dict.cend()) {
    // json not contains key "Name", no need process
    return false;
  }
  bool changed = false;
  static const auto& pm = CastManager::getIns();
  const QString& name = nameIt.value().toString();
  auto perfIt = dict.find(ENUM_TO_STRING(Cast));
  if (perfIt == dict.cend() || perfIt.value().toStringList().isEmpty()) {
    dict.insert(ENUM_TO_STRING(Cast), pm(name));
    changed = true;
  }
  static const auto& psm = StudiosManager::getIns();
  auto studioIt = dict.find(ENUM_TO_STRING(Studio));
  if (studioIt == dict.cend() || studioIt.value().toString().isEmpty()) {
    dict.insert(ENUM_TO_STRING(Studio), psm(name));
    changed = true;
  }
  return changed;
}

AppendPerfsToDict::AppendPerfsToDict(const QString& perfsStr)  //
    : performerList{NameTool()(perfsStr)}                      //
{}

bool AppendPerfsToDict::operator()(QVariantHash& dict) const {
  auto perfIt = dict.find(ENUM_TO_STRING(Cast));
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
    dict[ENUM_TO_STRING(Cast)] = afterLst;
  }
  return true;
}

bool UpdateStudio::operator()(QVariantHash& dict) const {
  auto studioIt = dict.find(ENUM_TO_STRING(Studio));
  if (studioIt == dict.cend()) {
    dict[ENUM_TO_STRING(Studio)] = m_studio;
    return true;
  }
  if (studioIt.value().toString() == m_studio) {
    return false;
  }
  studioIt.value() = m_studio;
  return true;
}

bool StandardlizeJsonKey::operator()(QVariantHash& dict) const {
  // Studio,xperf,Cast,Tags,Rate,Hot,Duration
  bool bHasChanged{false};
  auto it = dict.find(ENUM_TO_STRING(Performers));
  if (it != dict.cend()) {
    QStringList castsLst = it.value().toStringList();
    dict.erase(it);
    dict[ENUM_TO_STRING(Cast)] = castsLst;  // Perf removed, cast may update or not
    bHasChanged = true;
  }
  if (dict.find(ENUM_TO_STRING(Cast)) == dict.cend()) {
    dict[ENUM_TO_STRING(Cast)] = QStringList{};  // Cast added
    bHasChanged = true;
  }
  if (dict.find(ENUM_TO_STRING(Studio)) == dict.cend()) {
    dict[ENUM_TO_STRING(Studio)] = QString{};  // Studio added
    bHasChanged = true;
  }
  if (dict.find(ENUM_TO_STRING(Tags)) == dict.cend()) {
    dict[ENUM_TO_STRING(Tags)] = QStringList{};  // Tags added
    bHasChanged = true;
  }
  if (dict.find(ENUM_TO_STRING(Rate)) == dict.cend()) {
    dict[ENUM_TO_STRING(Rate)] = 0;  // Rate added
    bHasChanged = true;
  }
  if (dict.find(ENUM_TO_STRING(Hot)) == dict.cend()) {
    dict[ENUM_TO_STRING(Hot)] = QVariantList{};  // Hot added
    bHasChanged = true;
  }
  if (dict.find(ENUM_TO_STRING(Duration)) == dict.cend()) {
    dict[ENUM_TO_STRING(Duration)] = 0;  // Duration added
    bHasChanged = true;
  }
  return bHasChanged;
}

}  // namespace DictEditOperator

namespace JsonFileHelper {
using namespace DictEditOperator;
using namespace JSON_KEY;
QVariantHash GetJsonDictByMovieFile(const QString& vidFilePth, const QString& castStr, const QString& studio) {
  static const NameTool nt;
  const QStringList& performersList = nt(castStr);
  const QFileInfo fi{vidFilePth};
  return QVariantHash{
      {ENUM_TO_STRING(Name), fi.baseName()},                            //
      {ENUM_TO_STRING(Cast), performersList},                           //
      {ENUM_TO_STRING(Studio), studio},                                 //
      {ENUM_TO_STRING(Uploaded), fi.birthTime().toString("yyyyMMdd")},  //
      {ENUM_TO_STRING(Tags), JSON_DEF_VAL_Tags},                        //
      {ENUM_TO_STRING(Rate), JSON_DEF_VAL_Rate},                        //
      {ENUM_TO_STRING(Size), fi.size()},                                //
      {ENUM_TO_STRING(Resolution), JSON_DEF_VAL_Resolution},            //
      {ENUM_TO_STRING(Bitrate), JSON_DEF_VAL_Bitrate},                  //
      {ENUM_TO_STRING(Hot), JSON_DEF_VAL_Hot},                          //
      {ENUM_TO_STRING(Detail), JSON_DEF_VAL_Detail},                    //
      {ENUM_TO_STRING(Duration), JSON_DEF_VAL_Duration}                 //
  };
}

QVariantHash GetJsonDictDefault(const QString& vidName, const qint64& fileSz) {
  return QVariantHash{
      {ENUM_TO_STRING(Name), vidName},                        //
      {ENUM_TO_STRING(Cast), JSON_DEF_VAL_Cast},              //
      {ENUM_TO_STRING(Studio), JSON_DEF_VAL_Studio},          //
      {ENUM_TO_STRING(Uploaded), JSON_DEF_VAL_Uploaded},      //
      {ENUM_TO_STRING(Tags), JSON_DEF_VAL_Tags},              //
      {ENUM_TO_STRING(Rate), JSON_DEF_VAL_Rate},              //
      {ENUM_TO_STRING(Size), fileSz},                         //
      {ENUM_TO_STRING(Resolution), JSON_DEF_VAL_Resolution},  //
      {ENUM_TO_STRING(Bitrate), JSON_DEF_VAL_Bitrate},        //
      {ENUM_TO_STRING(Hot), JSON_DEF_VAL_Hot},                //
      {ENUM_TO_STRING(Detail), JSON_DEF_VAL_Detail},          //
      {ENUM_TO_STRING(Duration), JSON_DEF_VAL_Duration}       //
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

int JsonFileKeyValueProcess(const QString& path, const DictEditOperator::JSON_DICT_PROCESS_T jDProc) {
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

QVariantHash DeserializedJsonStr2Dict(const QString& serializedJsonStr) {
  if (serializedJsonStr.isEmpty()) {
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
    it = dict.find(ENUM_TO_STRING(Duration));  // here size is the duration
    if (it != dict.cend() && it.value().toInt() != duration) {
      it->setValue(duration);
      changed = true;
    }
  }
  if (!studio.isEmpty()) {
    it = dict.find(ENUM_TO_STRING(Studio));
    if (it != dict.cend() && it.value().toString() != studio) {
      it->setValue(studio);
      changed = true;
    }
  }
  if (!cast.isEmpty()) {
    const QStringList& castLst = cast.split(ELEMENT_JOINER);  // casts must seperated by comma only
    it = dict.find(ENUM_TO_STRING(Cast));                     // here cast is the Performers
    if (it != dict.cend() && it.value().toStringList() != castLst) {
      it->setValue(castLst);
      changed = true;
    }
  }
  if (!tags.isEmpty()) {
    const QStringList& tagsLst = tags.split(ELEMENT_JOINER);  // tags must seperated by comma only
    it = dict.find(ENUM_TO_STRING(Tags));
    if (it != dict.cend() && it.value().toStringList() != tagsLst) {
      it->setValue(tagsLst);
      changed = true;
    }
  }

  if (!changed) {
    return NOCHANGED_OK;
  }
  if (!DumpJsonDict(dict, jsonPth)) {
    qWarning("json[%s] dump failed", qPrintable(jsonPth));
    return CHANGED_WRITE_FILE_FAILED;
  }
  return CHANGED_OK;
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
    const QString& studio = dict.value(ENUM_TO_STRING(Studio), "").toString();
    if (studio.isEmpty()) {
      continue;
    }
    const QStringList& cast = dict.value(ENUM_TO_STRING(Cast), {}).toStringList();
    if (cast.isEmpty()) {
      continue;
    }
    const QStringList& tags = dict.value(ENUM_TO_STRING(Tags), {}).toStringList();
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
