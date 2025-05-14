#include "JsonPr.h"
#include "public/PathTool.h"
#include "public/PublicMacro.h"
#include "Tools/CastManager.h"
#include "Tools/StudiosManager.h"
#include "Tools/NameTool.h"
#include "JsonHelper.h"
#include "public/PublicTool.h"
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

JsonPr::JsonPr(const QString& jsonAbsFile) {  //
  QString prepath;
  jsonFileName = PATHTOOL::GetPrepathAndFileName(jsonAbsFile, prepath);
  m_Prepath.swap(prepath);
  Reload();
}

bool JsonPr::Reload() {
  const QString absPth = GetAbsPath();
  if (!QFile::exists(absPth)) {
    qWarning("file[%s] not exist", qPrintable(absPth));
    return false;
  }
  const auto& json = JsonHelper::GetJsonObject(absPth);

  auto it = json.constFind(ENUM_TO_STRING(Name));
  if (it != json.constEnd()) {
    m_Name = it->toString();
  }

  // 1. standardlize Performer=>Cast
  it = json.constFind(ENUM_TO_STRING(CAST));
  if (it != json.constEnd()) {
    m_Cast.setBatch(it->toVariant().toStringList());
  } else {
    const auto performerIter = json.constFind(ENUM_TO_STRING(Performers));
    if (performerIter != json.constEnd()) {
      m_Cast.setBatch(performerIter->toVariant().toStringList());
    }
  }

  // 2. standardlize ProductionStudio=>Studio
  it = json.constFind(ENUM_TO_STRING(Studio));
  if (it != json.constEnd()) {
    m_Studio = it->toString();
  } else {
    const auto performerIter = json.constFind("ProductionStudio");
    if (performerIter != json.constEnd()) {
      m_Studio = performerIter->toString();
    }
  }

  it = json.constFind(ENUM_TO_STRING(Tags));
  if (it != json.constEnd()) {
    m_Tags.setBatch(it->toVariant().toStringList());
  }

  it = json.constFind(ENUM_TO_STRING(Detail));
  if (it != json.constEnd()) {
    m_Detail = it->toString();
  }

  it = json.constFind(ENUM_TO_STRING(Uploaded));
  if (it != json.constEnd()) {
    m_Uploaded = it->toString();
  }

  it = json.constFind(ENUM_TO_STRING(Rate));
  if (it != json.constEnd()) {
    m_Rate = it->toInt();
  }

  it = json.constFind(ENUM_TO_STRING(Size));
  if (it != json.constEnd()) {
    m_Size = it->toInt();
  }

  it = json.constFind(ENUM_TO_STRING(Resolution));
  if (it != json.constEnd()) {
    m_Resolution = it->toString();
  }

  it = json.constFind(ENUM_TO_STRING(Bitrate));
  if (it != json.constEnd()) {
    m_Bitrate = it->toString();
  }

  it = json.constFind(ENUM_TO_STRING(Hot));
  if (it != json.constEnd()) {
    m_Hot.setBatch(it->toVariant().toStringList());
  }

  it = json.constFind(ENUM_TO_STRING(Duration));
  if (it != json.constEnd()) {
    m_Bitrate = it->toInt();
  }
  return true;
}

bool JsonPr::WriteIntoFiles() const {
  const QString& jsonPath = GetAbsPath();
  if (!QFile::exists(jsonPath)) {
    return false;
  }
  QJsonObject json;
  json[ENUM_TO_STRING(Name)] = m_Name;
  json[ENUM_TO_STRING(Cast)] = QJsonArray::fromStringList(m_Cast.toSortedList());
  json[ENUM_TO_STRING(Studio)] = m_Studio;
  json[ENUM_TO_STRING(Tags)] = QJsonArray::fromStringList(m_Tags.toSortedList());
  json[ENUM_TO_STRING(Detail)] = m_Detail;
  json[ENUM_TO_STRING(Uploaded)] = m_Uploaded;
  json[ENUM_TO_STRING(Rate)] = m_Rate;
  json[ENUM_TO_STRING(Size)] = m_Size;
  json[ENUM_TO_STRING(Resolution)] = m_Resolution;
  json[ENUM_TO_STRING(Bitrate)] = m_Bitrate;
  json[ENUM_TO_STRING(Hot)] = QJsonArray::fromStringList(m_Hot.toSortedList());
  json[ENUM_TO_STRING(Duration)] = m_Duration;
  const QByteArray& ba = QJsonDocument(json).toJson(QJsonDocument::Indented);
  return ByteArrayWriter(jsonPath, ba);
}

int JsonPr::Rename(const QString& newJsonNameUserInput, bool alsoRenameRelatedFiles) {
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
  if (!dir.rename(jsonFileName, newJsonName)) {
    qWarning("Rename json failed[%s]->[%s]", qPrintable(jsonFileName), qPrintable(newJsonName));
    return E_JSON_FILE_RENAME_FAILED;
  }
  int renameCnt = 1;
  if (!alsoRenameRelatedFiles) {
    return renameCnt;
  }
  const QString& oldJsonBaseName{PATHTOOL::GetBaseName(jsonFileName)};
  QString fileName, fileExt;
  QString newFileName;

  for (const QString& oldName : dir.entryList()) {
    std::tie(fileName, fileExt) = PATHTOOL::GetBaseNameExt(oldName);
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
  const QString newbaseName{PATHTOOL::GetBaseName(jsonFileName)};
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
  static const auto& pm = CastManager::getIns();
  if (m_Cast.isEmpty()) {
    m_Cast.setBatch(pm(m_Name));
    changed = true;
  }
  static const auto& psm = StudiosManager::getIns();
  if (m_Studio.isEmpty()) {
    m_Studio = psm(m_Name);
    changed = true;
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
  SortedUniqueStrContainer* p2Lst{nullptr};
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

void JsonPr::HintForCastStudio(const QString& selectedText) const {
  static StudiosManager& psm = StudiosManager::getIns();
  hintStudio = psm[m_Name];

  static CastManager& pm = CastManager::getIns();
  const QStringList& hintPerfsList = pm(m_Name + " " + selectedText);
  QSet<QString> elseCastSet{hintPerfsList.cbegin(), hintPerfsList.cend()};
  elseCastSet.subtract(m_Cast.m_set);
  hintCast = elseCastSet.values().join(NameTool::CSV_COMMA);
}

void JsonPr::AcceptCastHint() {
  if (hintCast.isEmpty()) {
    qDebug("hintCast is empty, skip accept cast hint");
    return;
  }
  m_Cast.insertBatchFromSentence(hintCast);
  hintCast.clear();
}

void JsonPr::AcceptStudioHint() {
  if (hintStudio.isEmpty()) {
    qDebug("hintStudio is empty, skip accept studio hint");
    return;
  }
  m_Studio = hintStudio;
  hintStudio.clear();
}

void JsonPr::RejectCastHint() {
  hintCast.clear();
}

void JsonPr::RejectStudioHint() {
  hintStudio.clear();
}
