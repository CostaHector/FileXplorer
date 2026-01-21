#include "JsonHelper.h"
#include "JsonKey.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "PublicTool.h"
#include "PublicMacro.h"
#include "NameTool.h"
#include "CastManager.h"
#include "StudiosManager.h"
#include "TableFields.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDirIterator>
#include <QFile>

namespace JsonHelper {
QByteArray SerializedJsonDict2ByteArray(const QVariantHash& dict) {
  if (dict.isEmpty()) {return "{}";}   // JSON标准兼容性——RFC7159
  const QJsonObject& jsonObject{QJsonObject::fromVariantHash(dict)};
  QJsonDocument document;
  document.setObject(jsonObject);
  return document.toJson(QJsonDocument::JsonFormat::Indented);
}

QVariantHash DeserializedJsonByteArray2Dict(const QByteArray& jsonBa, bool* bParseOk) {
  if (jsonBa.isEmpty()) { // JSON标准兼容性——RFC7159
    if (bParseOk) *bParseOk = false;
    LOG_W("Deserialized empty ByteArray failed");
    return {};
  }
  if (jsonBa.simplified() == "{}") {   // 尝试解析最小化形式（无空白）的空对象
    if (bParseOk) *bParseOk = true;
    return {};
  }
  QJsonParseError jsonErr;
  const QJsonDocument json_doc = QJsonDocument::fromJson(jsonBa, &jsonErr);
  if (bParseOk != nullptr) {
    *bParseOk = jsonErr.error == QJsonParseError::NoError;
  }
  if (jsonErr.error != QJsonParseError::NoError) {
    LOG_W("Deserialized %d ByteArray failed. [code:%d, msg: %s]", jsonBa.size(), (int)jsonErr.error, qPrintable(jsonErr.errorString()));
    return {};
  }
  QJsonObject jsonObject = json_doc.object();
  return jsonObject.toVariantHash();
}

QVariantHash DeserializedJsonStr2Dict(const QString& serializedJsonStr, bool* bParseOk) {
  return DeserializedJsonByteArray2Dict(serializedJsonStr.toUtf8(), bParseOk);
}

bool DumpJsonDict(const QVariantHash& dict, const QString& jsonFilePth) {
  const QByteArray& byteArray = SerializedJsonDict2ByteArray(dict);
  return FileTool::ByteArrayWriter(jsonFilePth, byteArray);
}

QVariantHash MovieJsonLoader(const QString& jsonFilePth) {
  const QString& json_string = FileTool::TextReader(jsonFilePth);
  return DeserializedJsonStr2Dict(json_string);
}

QJsonObject GetJsonObject(const QString& filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    LOG_W("Failed to open file: %s", qPrintable(filePath));
    return {};
  }
  QByteArray jsonData = file.readAll();
  file.close();
  if (jsonData.isEmpty()) {
    return {};
  }

  QJsonParseError jsonErr;
  QJsonDocument json_doc = QJsonDocument::fromJson(jsonData, &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError) {
    LOG_W("Error parse json string %d char(s): %s", jsonData.size(), qPrintable(jsonErr.errorString()));
    return {};
  }
  return json_doc.object();
}

RET_ENUM InsertOrUpdateDurationStudioCastTags(const QString& jsonPth, int duration, const QString& studio, const QString& cast, const QString& tags) {
  QVariantHash dict;
  bool changed{false};
  if (QFile::exists(jsonPth)) {
    dict = MovieJsonLoader(jsonPth);
  } else {
    const QString& name = PathTool::GetBaseName(jsonPth);
    dict = JsonKey::GetJsonDictDefault(name);
    changed = true;
  }

  QHash<QString, QVariant>::iterator it;
  if (duration != 0) {
    it = dict.find(ENUM_2_STR(Duration));  // here size is the duration
    if (it != dict.cend() && it->toInt() != duration) {
      it->setValue(duration);
      changed = true;
    }
  }
  if (!studio.isEmpty()) {
    it = dict.find(ENUM_2_STR(Studio));
    if (it != dict.cend() && it->toString() != studio) {
      it->setValue(studio);
      changed = true;
    }
  }
  if (!cast.isEmpty()) {
    const QStringList& castLst = cast.split(ELEMENT_JOINER);  // casts must seperated by comma only
    it = dict.find(ENUM_2_STR(Cast));                     // here cast is the Performers
    if (it != dict.cend() && it->toStringList() != castLst) {
      it->setValue(castLst);
      changed = true;
    }
  }
  if (!tags.isEmpty()) {
    const QStringList& tagsLst = tags.split(ELEMENT_JOINER);  // tags must seperated by comma only
    it = dict.find(ENUM_2_STR(Tags));
    if (it != dict.cend() && it->toStringList() != tagsLst) {
      it->setValue(tagsLst);
      changed = true;
    }
  }

  if (!changed) {
    return NOCHANGED_OK;
  }
  if (!DumpJsonDict(dict, jsonPth)) {
    LOG_W("json[%s] dump failed", qPrintable(jsonPth));
    return CHANGED_WRITE_FILE_FAILED;
  }
  return CHANGED_OK;
}

QMap<uint, JsonDict2Table> ReadStudioCastTagsOut(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    LOG_D("path[%s] is not a dir", qPrintable(path));
    return {};
  }

  QMap<uint, JsonDict2Table> fileNameHash2Json;
  QDirIterator it{path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = MovieJsonLoader(jsonPath);
    using namespace MOVIE_TABLE;
    const QString& studio = dict.value(ENUM_2_STR(Studio), "").toString();
    const QStringList& cast = dict.value(ENUM_2_STR(Cast), {}).toStringList();
    const QStringList& tags = dict.value(ENUM_2_STR(Tags), {}).toStringList();
    if (studio.isEmpty() && cast.isEmpty() && tags.isEmpty()) {
      continue;
    }
    fileNameHash2Json[CalcFileHash(jsonPath)] = JsonDict2Table{studio, cast, tags};
  }
  LOG_D("%d json file contains Studio|Cast|Tags Field", fileNameHash2Json.size());
  return fileNameHash2Json;
}

uint CalcFileHash(const QString& vidPth) {
  return qHash(PathTool::GetFileNameExtRemoved(vidPth));
}

}  // namespace JsonHelper
