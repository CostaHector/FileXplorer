#include "JsonPr.h"
#include "PathTool.h"
#include "PublicMacro.h"
#include "CastManager.h"
#include "StudiosManager.h"
#include "NameTool.h"
#include "JsonHelper.h"
#include "FileTool.h"
#include "PublicVariable.h"
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "DataFormatter.h"
#include "VideoDurationGetter.h"
#include "DvdFileInfo.h"
#include "MD5Calculator.h"
#include "JsonParser.h"
#include "JsonUpdater.h"

JsonPr JsonPr::fromJsonFile(const QString& jsonAbsFile) {
  QString prepath;
  QString jsonFileName = PathTool::GetPrepathAndFileName(jsonAbsFile, prepath);
  const QJsonObject& json = JsonHelper::GetJsonObject(jsonAbsFile);
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
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, generalDataType, format, writer, initer, jsonWriter) m_##enu{initer(json, ENUM_2_STR(enu), defValue)},
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
      jsonFileName{fileName} {  //
  m_Detail.replace("<br/>", "\n");

  const QString& jsonFileBaseName = PathTool::GetBaseName(jsonFileName);
  QString txtAbsPath = PathTool::Path2Join(m_Prepath, jsonFileBaseName+".txt");
  JsonUpdater::MergeTextContentsIntoDetailAndRecycleTxt(txtAbsPath, m_Detail);
}

bool JsonPr::operator==(const JsonPr& rhs) const {
  return
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, generalDataType, format, writer, initer, jsonWriter) m_##enu == rhs.m_##enu&&
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
          jsonFileName == rhs.jsonFileName;
}

bool JsonPr::operator<(const JsonPr& rhs) const {
  return jsonFileName < rhs.jsonFileName;
}

bool JsonPr::Reload() {
  const QString absPth = GetJsonFileAbsPath();
  if (!QFileInfo(absPth).isFile()) {
    LOG_W("file[%s] not exist", qPrintable(absPth));
    return false;
  }
  const auto& json = JsonHelper::GetJsonObject(absPth);
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, generalDataType, format, writer, initer, jsonWriter) m_##enu = initer(json, ENUM_2_STR(enu), defValue);
  JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
  m_Detail.replace("<br/>", "\n");
  return true;
}

bool JsonPr::WriteIntoFiles() const {
  const QString& jsonPath = GetJsonFileAbsPath();
  if (!QFile::exists(jsonPath)) {
    return false;
  }
  const QByteArray& ba{GetJsonBA()};
  bool writeResult{FileTool::ByteArrayTextWriter(jsonPath, ba)};
  if (writeResult) {
    hintCast.clear();
    hintStudio.clear();
  }
  return writeResult;
}

QByteArray JsonPr::GetJsonBA() const {
  QJsonObject json;
#define JSON_KEY_ITEM(enu, enumVal, defValue, enhanceDefVal, generalDataType, format, writer, initer, jsonWriter) jsonWriter(json, ENUM_2_STR(enu), m_##enu);
  JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
  return QJsonDocument(json).toJson(QJsonDocument::Indented);
}

QStringList JsonPr::GetImagesAbsPath() const {
  QStringList imgs;
  imgs.reserve(m_ImgName.size());
  for (const QString& imgName: m_ImgName) {
    imgs.push_back(GetItemsAbsPath(imgName));
  }
  return imgs;
}

QString JsonPr::GetVideoAbsPath() const {
  if (m_VidName.isEmpty()) {
    return "";
  }
  return GetItemsAbsPath(m_VidName);
}

bool JsonPr::SyncNameValueFromFileBaseName() {
  QString newbaseName{PathTool::GetBaseName(jsonFileName)};
  if (newbaseName == m_Name) { // no need update
    return false;
  }
  m_Name.swap(newbaseName);
  return true;
}

QString JsonPr::FindVideoAbsPath() const {
  const QString& jsonFileBaseName = PathTool::GetBaseName(jsonFileName);
  QString videoAbsPath = m_Prepath + '/' + jsonFileBaseName;
  const int beforeSize = videoAbsPath.size();
  videoAbsPath.reserve(beforeSize + 7);
  for (const QString& ext: TYPE_FILTER::VIDEO_TYPE_SET) {
    videoAbsPath += ext.midRef(1);
    if (QFile::exists(videoAbsPath)) {
      return videoAbsPath;
    }
    videoAbsPath.chop(videoAbsPath.size() - beforeSize);
  }
  videoAbsPath.chop(videoAbsPath.size() - beforeSize);
  return "";
}

bool JsonPr::UpdateVideoSizeField(QString videoAbsPath) {
  if (videoAbsPath.isEmpty()) {
    videoAbsPath = FindVideoAbsPath();
  }
  if (!QFile::exists(videoAbsPath)) {
    LOG_D("Video correspond to json file[%s] not found", qPrintable(jsonFileName));
    return false;
  }
  qint64 newSize = FileTool::GetFileSize(videoAbsPath);
  if (m_Size == newSize) {
    return false;
  }
  m_Size = newSize;
  return true;
}

bool JsonPr::UpdateDurationField(QString videoAbsPath) {
  if (videoAbsPath.isEmpty()) {
    videoAbsPath = FindVideoAbsPath();
  }
  if (!QFile::exists(videoAbsPath)) {
    LOG_D("Video correspond to json file[%s] not found", qPrintable(jsonFileName));
    return false;
  }
  VideoDurationGetter mi;
  int newDuration = VideoDurationGetter::GetLengthQuickStatic(mi, videoAbsPath);
  if (newDuration <= 0) {
    return false;
  }
  m_Duration = newDuration;
  return true;
}

bool JsonPr::UpdateVideoMD5Field(QString videoAbsPath) {
  if (videoAbsPath.isEmpty()) {
    videoAbsPath = FindVideoAbsPath();
  }
  if (!QFile::exists(videoAbsPath)) {
    LOG_D("Video correspond to json file[%s] not found", qPrintable(jsonFileName));
    return false;
  }

  if (videoAbsPath.endsWith(".dvd", Qt::CaseInsensitive)) {
    m_MD5 = DvdFileInfo::ReadTotalMD5FromDvdFile(videoAbsPath);
  } else {
    m_MD5 = MD5Calculator::GetFileMD5(videoAbsPath, BytesRangeTool::BytesRangeE::SAMPLED_128_KB);
  }
  return true;
}

bool JsonPr::ConstructCastStudioValue() {
  if (m_Name.isEmpty()) {
    return false;
  }
  bool changed = false;

  static const StudiosManager& studioMgr = StudiosManager::getInst();
  bool bIsActorFromSingleWordStudio = false;
  if (m_Studio.isEmpty()) {
    QString newStudio = studioMgr(m_Name);
    if (!newStudio.isEmpty()) {
      m_Studio.swap(newStudio);
      changed = true;
    }
    bIsActorFromSingleWordStudio = studioMgr.isStudioWithSingleWord(hintStudio);
  } else {
    bIsActorFromSingleWordStudio = studioMgr.isStudioWithSingleWord(m_Studio);
  }

  if (m_Cast.isEmpty()) {
    static const CastManager& actorMgr = CastManager::getInst();
    QStringList newCastLst = actorMgr(m_Name, bIsActorFromSingleWordStudio);
    if (!newCastLst.isEmpty()) {
      m_Cast.setBatch(newCastLst);
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

bool JsonPr::SetCastOrTags(const QString& val, JsonModelField::FIELD_OP_TYPE fieldType, JsonModelField::FIELD_OP_MODE fieldMode) {
  SortedUniqStrLst* p2Lst{nullptr};
  switch (fieldType) {
    case JsonModelField::FIELD_OP_TYPE::CAST:
      p2Lst = &m_Cast;
      break;
    case JsonModelField::FIELD_OP_TYPE::TAGS:
      p2Lst = &m_Tags;
      break;
    default:
      LOG_W("Field type[%d] invalid", (int)fieldType);
      return false;
  }
  if (p2Lst == nullptr) {
    LOG_W("Field type[%d] not exist in json dict", (int)fieldType);
    return false;
  }

  switch (fieldMode) {
    case JsonModelField::FIELD_OP_MODE::SET: {
      p2Lst->setBatchFromSentence(val);
      break;
    }
    case JsonModelField::FIELD_OP_MODE::APPEND: {
      p2Lst->insertBatchFromSentence(val);
      break;
    }
    case JsonModelField::FIELD_OP_MODE::REMOVE: {
      p2Lst->remove(val);
      break;
    }
    default:
      LOG_W("Field Operation Mode[%d] invalid", (int)fieldMode);
      return false;
  }
  return true;
}

void JsonPr::HintForCastStudio(const QString& selectedText, bool& studioChanged, bool& castChanged) const {
  static StudiosManager& studioMgr = StudiosManager::getInst();
  hintStudio = studioMgr(m_Name);
  bool bIsActorFromSingleWordStudio = false;
  if (!hintStudio.isEmpty()) {
    bIsActorFromSingleWordStudio = studioMgr.isStudioWithSingleWord(hintStudio);
    if (m_Studio != hintStudio) { // not equal update
      studioChanged = true;
    } else {
      studioChanged = false;
      hintStudio.clear();
    }
  } else {
    studioChanged = false;
    bIsActorFromSingleWordStudio = studioMgr.isStudioWithSingleWord(m_Studio);
  }

  static CastManager& actorMgr = CastManager::getInst();
  const QStringList& hintPerfsList = actorMgr(m_Name + " " + selectedText, bIsActorFromSingleWordStudio);
  QSet<QString> elseCastSet{hintPerfsList.cbegin(), hintPerfsList.cend()};
  elseCastSet.subtract(m_Cast.m_set); // has increasing update
  QStringList elseCastList{elseCastSet.values()};
  std::sort(elseCastList.begin(), elseCastList.end());
  hintCast = elseCastList.join(NameTool::CSV_COMMA);
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

bool JsonPr::checkMd5AndVidNameConsistency() const {
  // return true if MD5 and VidName both empty or both non-empty.
  return JsonParser::ValidateSampleMd5AndVidName(m_MD5.size(), m_VidName.size()) != JsonOp::ResultE::ERROR;
}