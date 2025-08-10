#include "JsonPr.h"
#include "PathTool.h"
#include "PublicMacro.h"
#include "CastManager.h"
#include "StudiosManager.h"
#include "NameTool.h"
#include "JsonHelper.h"
#include "PublicTool.h"
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "DataFormatter.h"

JsonPr JsonPr::fromJsonFile(const QString& jsonAbsFile) {
  QString prepath;
  QString jsonFileName = PathTool::GetPrepathAndFileName(jsonAbsFile, prepath);
  const auto& json = JsonHelper::GetJsonObject(jsonAbsFile);
  return JsonPr{prepath, jsonFileName, json};
}

JsonPr::JsonPr(const QString& jsonAbsFile) {  //
  QString prepath;
  jsonFileName = PathTool::GetPrepathAndFileName(jsonAbsFile, prepath);
  m_Prepath.swap(prepath);
  Reload();
}

JsonPr::JsonPr(const QString& filePrePath, const QString& fileName, const QJsonObject& json)  //
    : m_Prepath{filePrePath},                                                                 //
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, format, writer, initer, jsonWriter) m_##enu{initer(json, ENUM_2_STR(enu), defValue)},
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
      jsonFileName{fileName} {  //
  m_Detail.replace("<br/>", "\n");
}

bool JsonPr::operator==(const JsonPr& rhs) const {
  return
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, format, writer, initer, jsonWriter) m_##enu == rhs.m_##enu&&
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
          jsonFileName == rhs.jsonFileName;
}

bool JsonPr::Reload() {
  const QString absPth = GetAbsPath();
  if (!QFile::exists(absPth)) {
    qWarning("file[%s] not exist", qPrintable(absPth));
    return false;
  }
  const auto& json = JsonHelper::GetJsonObject(absPth);
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, format, writer, initer, jsonWriter) m_##enu = initer(json, ENUM_2_STR(enu), defValue);
  JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
  m_Detail.replace("<br/>", "\n");
  return true;
}

bool JsonPr::WriteIntoFiles() const {
  const QString& jsonPath = GetAbsPath();
  if (!QFile::exists(jsonPath)) {
    return false;
  }
  const QByteArray& ba{GetJsonBA()};
  bool writeResult{ByteArrayWriter(jsonPath, ba)};
  if (writeResult) {
    hintCast.clear();
    hintStudio.clear();
  }
  return writeResult;
}

QByteArray JsonPr::GetJsonBA() const {
  QJsonObject json;
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, format, writer, initer, jsonWriter) jsonWriter(json, ENUM_2_STR(enu), m_##enu);
  JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
  return QJsonDocument(json).toJson(QJsonDocument::Indented);
}

int JsonPr::RenameJsonAndRelated(const QString& newJsonNameUserInput, bool alsoRenameRelatedFiles) {
  QString newJsonName{newJsonNameUserInput};
  QString newJsonBaseName{newJsonName};
  if (!newJsonName.endsWith(JsonHelper::JSON_EXT)) {
    newJsonName += JsonHelper::JSON_EXT;
  } else {
    newJsonBaseName.chop(JsonHelper::JSON_EXT_LENGTH);
  }
  if (newJsonName == jsonFileName) {
    return E_OK;  // skip
  }

  QDir dir{m_Prepath, "", QDir::SortFlag::Name, QDir::Filter::Files};
  if (!dir.exists(jsonFileName)) {
    return E_JSON_NOT_EXIST;
  }
  if (dir.exists(newJsonName)) {
    return E_JSON_NEW_NAME_OCCUPID;
  }
  if (!dir.rename(jsonFileName, newJsonName)) {
    qWarning("Rename json failed[%s]->[%s]", qPrintable(jsonFileName), qPrintable(newJsonName));
    return E_JSON_FILE_RENAME_FAILED;
  }
  int renameCnt = 1;
  if (!alsoRenameRelatedFiles) {
    return renameCnt;
  }

  QString fileName, fileExt;
  QString newFileName;
  const QString& oldJsonBaseName{PathTool::GetBaseName(jsonFileName)};
  for (const QString& oldName : dir.entryList({oldJsonBaseName + "*"})) {
    std::tie(fileName, fileExt) = PathTool::GetBaseNameExt(oldName);
    if (fileName.leftRef(oldJsonBaseName.size()) != oldJsonBaseName) {
      continue;
    }
    newFileName = newJsonBaseName + oldName.mid(oldJsonBaseName.size());
    if (!dir.rename(oldName, newFileName)) {
      qWarning("Rename related failed[%s]->[%s]", qPrintable(oldName), qPrintable(newFileName));
      return E_RELATED_FILE_RENAME_FAILED;
    }
    ++renameCnt;
  }
  return renameCnt;
}

bool JsonPr::SyncNameValueFromFileBaseName() {
  const QString newbaseName{PathTool::GetBaseName(jsonFileName)};
  if (newbaseName == m_Name) {
    return false;
  }
  m_Name = newbaseName;
  return true;
}

bool JsonPr::ConstructCastStudioValue() {
  if (m_Name.isEmpty()) {
    return false;
  }
  bool changed = false;
  if (m_Cast.isEmpty()) {
    static const auto& pm = CastManager::getIns();
    QStringList newCastLst = pm(m_Name);
    if (!newCastLst.isEmpty()) {
      m_Cast.setBatch(newCastLst);
      changed = true;
    }
  }
  if (m_Studio.isEmpty()) {
    static const auto& psm = StudiosManager::getIns();
    QString newStudio = psm(m_Name);
    if (!newStudio.isEmpty()) {
      m_Studio.swap(newStudio);
      changed = true;
    }
  }
  return changed;
}

bool JsonPr::ClearCastStudioValue() {
  bool changed{false};
  if (!m_Studio.isEmpty()) {
    m_Studio.clear();
    changed = true;
  }
  if (!m_Cast.isEmpty()) {
    m_Cast.clear();
    changed = true;
  }
  return changed;
}

bool JsonPr::SetStudio(const QString& studio) {
  if (m_Studio == studio) {
    return false;
  }
  m_Studio = studio;
  return true;
}

bool JsonPr::SetCastOrTags(const QString& val, FIELD_OP_TYPE fieldType, FIELD_OP_MODE fieldMode) {
  SortedUniqStrLst* p2Lst{nullptr};
  switch (fieldType) {
    case FIELD_OP_TYPE::CAST:
      p2Lst = &m_Cast;
      break;
    case FIELD_OP_TYPE::TAGS:
      p2Lst = &m_Tags;
      break;
    default:
      qWarning("Field type[%d] invalid", (int)fieldType);
      return false;
  }
  if (p2Lst == nullptr) {
    qWarning("Field type[%d] not exist in json dict", (int)fieldType);
    return false;
  }

  switch (fieldMode) {
    case FIELD_OP_MODE::SET: {
      p2Lst->setBatchFromSentence(val);
      break;
    }
    case FIELD_OP_MODE::APPEND: {
      p2Lst->insertBatchFromSentence(val);
      break;
    }
    case FIELD_OP_MODE::REMOVE: {
      p2Lst->remove(val);
      break;
    }
    default:
      qWarning("Field Operation Mode[%d] invalid", (int)fieldMode);
      return false;
  }
  return true;
}

void JsonPr::HintForCastStudio(const QString& selectedText, bool& studioChanged, bool& castChanged) const {
  static StudiosManager& psm = StudiosManager::getIns();
  hintStudio = psm(m_Name);
  if (m_Studio != hintStudio) {
    studioChanged = true;
  } else {
    studioChanged = false;
    hintStudio.clear();
  }

  static CastManager& pm = CastManager::getIns();
  const QStringList& hintPerfsList = pm(m_Name + " " + selectedText);
  QSet<QString> elseCastSet{hintPerfsList.cbegin(), hintPerfsList.cend()};
  elseCastSet.subtract(m_Cast.m_set);
  hintCast = elseCastSet.values().join(NameTool::CSV_COMMA);
  if (!elseCastSet.isEmpty()) {
    castChanged = true;
  } else {
    castChanged = false;
    hintCast.clear();
  }
}

void JsonPr::RejectCastHint() {
  hintCast.clear();
}

void JsonPr::RejectStudioHint() {
  hintStudio.clear();
}
