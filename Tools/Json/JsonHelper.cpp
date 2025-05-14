#include "JsonHelper.h"
#include "JsonKey.h"
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

bool ReCastAndStudio::operator()(QVariantHash& jsonDict) const {
  auto nameIt = jsonDict.constFind(ENUM_TO_STRING(Name));
  if (nameIt == jsonDict.cend()) {
    // json not contains key "Name", no need process
    return false;
  }
  static const auto& pm = CastManager::getIns();
  static const auto& psm = StudiosManager::getIns();

  const QString& name = nameIt->toString();
  bool changed = false;
  auto perfIt = jsonDict.find(ENUM_TO_STRING(Cast));
  if (perfIt != jsonDict.end()) {
    const QStringList cast = pm(name);
    if (cast != perfIt->toStringList()) {
      perfIt->setValue(cast);
      changed = true;
    }
  }

  auto studioIt = jsonDict.find(ENUM_TO_STRING(Studio));
  if (studioIt != jsonDict.end()) {
    const QString studio{psm(name)};
    if (studio != studioIt->toString()) {
      studioIt->setValue(psm(name));
      changed = true;
    }
  }
  return changed;
}

bool ConstructStudioCastByName::operator()(QVariantHash& jsonDict) const {
  auto nameIt = jsonDict.constFind(ENUM_TO_STRING(Name));
  if (nameIt == jsonDict.cend()) {
    // json not contains key "Name", no need process
    return false;
  }
  bool changed = false;
  static const auto& pm = CastManager::getIns();
  const QString& name = nameIt->toString();
  auto perfIt = jsonDict.find(ENUM_TO_STRING(Cast));
  if (perfIt == jsonDict.end() || perfIt->toStringList().isEmpty()) {
    jsonDict.insert(ENUM_TO_STRING(Cast), pm(name));
    changed = true;
  }
  static const auto& psm = StudiosManager::getIns();
  auto studioIt = jsonDict.find(ENUM_TO_STRING(Studio));
  if (studioIt == jsonDict.end() || studioIt->toString().isEmpty()) {
    jsonDict.insert(ENUM_TO_STRING(Studio), psm(name));
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
    afterLst += perfIt->toStringList();
    afterLst.sort();
    afterLst.removeDuplicates();
    if (afterLst == perfIt->toStringList()) {
      return false;
    }
    perfIt->setValue(afterLst);
  } else {
    afterLst.sort();
    afterLst.removeDuplicates();
    dict[ENUM_TO_STRING(Cast)] = afterLst;
  }
  return true;
}

bool UpdateStudio::operator()(QVariantHash& dict) const {
  auto studioIt = dict.find(ENUM_TO_STRING(Studio));
  if (studioIt == dict.end()) {
    dict[ENUM_TO_STRING(Studio)] = m_studio;
    return true;
  }
  if (studioIt->toString() == m_studio) {
    return false;
  }
  studioIt->setValue(m_studio);
  return true;
}

bool StandardlizeJsonKey::operator()(QVariantHash& jsonDict) const {
  // Studio,xperf,Cast,Tags,Rate,Hot,Duration
  bool bHasChanged{false};
  auto it = jsonDict.find(ENUM_TO_STRING(Performers));
  if (it != jsonDict.cend()) {
    QStringList castsLst = it->toStringList();
    jsonDict.erase(it);
    jsonDict[ENUM_TO_STRING(Cast)] = castsLst;  // Perf removed, cast may update or not
    bHasChanged = true;
  }
  if (jsonDict.constFind(ENUM_TO_STRING(Cast)) == jsonDict.cend()) {
    jsonDict[ENUM_TO_STRING(Cast)] = QStringList{};  // Cast added
    bHasChanged = true;
  }

  it = jsonDict.find("ProductionStudio");
  if (it != jsonDict.cend()) {
    QString studio = it->toString();
    jsonDict.erase(it);
    jsonDict[ENUM_TO_STRING(Studio)] = studio;  // ProductionStudio replaced by studio
    bHasChanged = true;
  } else if (jsonDict.constFind(ENUM_TO_STRING(Studio)) == jsonDict.cend()) {
    jsonDict[ENUM_TO_STRING(Studio)] = QString{};  // Studio added
    bHasChanged = true;
  }

  if (jsonDict.constFind(ENUM_TO_STRING(Tags)) == jsonDict.cend()) {
    jsonDict[ENUM_TO_STRING(Tags)] = QStringList{};  // Tags added
    bHasChanged = true;
  }
  if (jsonDict.constFind(ENUM_TO_STRING(Rate)) == jsonDict.cend()) {
    jsonDict[ENUM_TO_STRING(Rate)] = 0;  // Rate added
    bHasChanged = true;
  }
  if (jsonDict.constFind(ENUM_TO_STRING(Hot)) == jsonDict.cend()) {
    jsonDict[ENUM_TO_STRING(Hot)] = QVariantList{};  // Hot added
    bHasChanged = true;
  }
  if (jsonDict.constFind(ENUM_TO_STRING(Duration)) == jsonDict.cend()) {
    jsonDict[ENUM_TO_STRING(Duration)] = 0;  // Duration added
    bHasChanged = true;
  }
  return bHasChanged;
}

}  // namespace DictEditOperator

namespace JsonHelper {
using namespace DictEditOperator;
bool DumpJsonDict(const QVariantHash& dict, const QString& jsonFilePth) {
  const auto& jsonObject{QJsonObject::fromVariantHash(dict)};
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

int SyncJsonNameValue(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a dir", qPrintable(path));
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;

  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    auto it = dict.find(ENUM_TO_STRING(Name));
    if (it == dict.cend()) {
      continue;
    }
    const QString& baseName = PATHTOOL::GetBaseName(jsonPath);
    if (it->toString() == baseName) {
      continue;
    }
    it->setValue(baseName);
    succeedCnt += DumpJsonDict(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json processed", succeedCnt, tryConstuctCnt);
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

  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
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

QJsonObject GetJsonObject(const QString& filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning("Failed to open file: %s", qPrintable(filePath));
    return {};
  }
  QByteArray jsonData = file.readAll();
  file.close();

  QJsonParseError jsonErr;
  QJsonDocument json_doc = QJsonDocument::fromJson(jsonData, &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError) {
    qWarning("Error parse json string %d char(s): %s", jsonData.size(), qPrintable(jsonErr.errorString()));
    return {};
  }
  return json_doc.object();
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
  return rootObj.toVariantHash();
}

RET_ENUM InsertOrUpdateDurationStudioCastTags(const QString& jsonPth, int duration, const QString& studio, const QString& cast, const QString& tags) {
  QVariantHash dict;
  bool changed{false};
  if (QFile::exists(jsonPth)) {
    dict = MovieJsonLoader(jsonPth);
  } else {
    const QString& name = PATHTOOL::GetBaseName(jsonPth);
    dict = JsonKey::GetJsonDictDefault(name);
    changed = true;
  }

  QHash<QString, QVariant>::iterator it;
  if (duration != 0) {
    it = dict.find(ENUM_TO_STRING(Duration));  // here size is the duration
    if (it != dict.cend() && it->toInt() != duration) {
      it->setValue(duration);
      changed = true;
    }
  }
  if (!studio.isEmpty()) {
    it = dict.find(ENUM_TO_STRING(Studio));
    if (it != dict.cend() && it->toString() != studio) {
      it->setValue(studio);
      changed = true;
    }
  }
  if (!cast.isEmpty()) {
    const QStringList& castLst = cast.split(ELEMENT_JOINER);  // casts must seperated by comma only
    it = dict.find(ENUM_TO_STRING(Cast));                     // here cast is the Performers
    if (it != dict.cend() && it->toStringList() != castLst) {
      it->setValue(castLst);
      changed = true;
    }
  }
  if (!tags.isEmpty()) {
    const QStringList& tagsLst = tags.split(ELEMENT_JOINER);  // tags must seperated by comma only
    it = dict.find(ENUM_TO_STRING(Tags));
    if (it != dict.cend() && it->toStringList() != tagsLst) {
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

int JsonSyncKeyValueAccordingJsonFileName(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a dir", qPrintable(path));
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;

  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    const QString& baseName = PATHTOOL::GetBaseName(jsonPath);
    auto nameIt = dict.find(ENUM_TO_STRING(Name));
    if (nameIt == dict.cend() || nameIt->toString() == baseName) {
      continue;
    }
    nameIt->setValue(baseName);
    succeedCnt += DumpJsonDict(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json processed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}

}  // namespace JsonHelper
