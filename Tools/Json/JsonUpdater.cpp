#include "JsonUpdater.h"
#include "JsonModelField.h"
#include "JsonFieldBoundary.h"
#include "Logger.h"
#include "PathTool.h"
#include "FileTool.h"
#include <QFileInfo>

namespace JsonUpdater {
bool MergeTextContentsIntoDetailAndRecycleTxt(const QString& txtAbsPath, QString& detailContent) {
  // return true if need detail changed
  if (!QFile::exists(txtAbsPath)) {
    return false;
  }
  if (FileTool::GetFileSize(txtAbsPath) > 10 * 1024) { // too big txt file(>10KiB)
    return false;
  }
  bool bReadOk{false};
  const QString contents = FileTool::StringTextReader(txtAbsPath, &bReadOk);
  if (!bReadOk) { // read failed
    return false;
  }
  if (detailContent.contains(contents)) {
    return false;
  }
  if (!detailContent.isEmpty()) {
    detailContent += '\n';
  }
  detailContent += contents;
#ifdef RUNNING_UNIT_TESTS
  QFile::remove(txtAbsPath);
#else
  QFile::moveToTrash(txtAbsPath);
#endif
  return true;
}

JsonOp::Counter UpdateJsonKeyValuePair(const ScenesMixed& sMixed, QVariantHash& rawJsonDict, const QString& parentPath, const QString& jsonFileBaseName) {
  // will update m_Name, m_ImgName, m_VidName, m_Size, m_Rate
  bool bJsonNeedUpdate{false}, bJsonUsed{false}, bImgNameUpdate{false}, bVidNameUpdated{false};
  if (rawJsonDict.isEmpty()) {
    LOG_W("json file[%s] may corrupt read failed", qPrintable(jsonFileBaseName));
    return JsonOp::Counter{(int)bJsonNeedUpdate, (int)bJsonUsed, (int)bImgNameUpdate, (int)bVidNameUpdated};
  }

  using namespace JsonModelField;
  QVariantHash::iterator it = rawJsonDict.find(ENUM_2_STR(Name));
  if (it == rawJsonDict.cend()) {
    LOG_D("This json file[%s] is not we want", qPrintable(jsonFileBaseName));
    return JsonOp::Counter{(int)bJsonNeedUpdate, (int)bJsonUsed, (int)bImgNameUpdate, (int)bVidNameUpdated};
  }

  bJsonUsed = true;
  if (it.value() != jsonFileBaseName) {
    it->setValue(jsonFileBaseName);
    bJsonNeedUpdate = true;
  }

  const QStringList& imgsLst{sMixed.GetAllImgs(jsonFileBaseName)};
  it = rawJsonDict.find(ENUM_2_STR(ImgName));
  if (it == rawJsonDict.cend()) {
    rawJsonDict.insert(ENUM_2_STR(ImgName), imgsLst);
    bJsonNeedUpdate = true;
    bImgNameUpdate = true;
  } else if (it.value().toStringList() != imgsLst) {
    it->setValue(imgsLst);
    bJsonNeedUpdate = true;
    bImgNameUpdate = true;
  }

  const QString& vidFileName{sMixed.GetFirstVid(jsonFileBaseName)};
  if (!vidFileName.isEmpty()) {
    it = rawJsonDict.find(ENUM_2_STR(VidName));
    if (it == rawJsonDict.cend()) {
      rawJsonDict.insert(ENUM_2_STR(VidName), vidFileName);
      bJsonNeedUpdate = true;
      bVidNameUpdated = true;
    } else {
      const QString& oldVidName = it.value().toString();
      if (oldVidName.isEmpty() || vidFileName != oldVidName) {
        it->setValue(vidFileName);
        bJsonNeedUpdate = true;
        bVidNameUpdated = true;
      }
    }

    const QString videoAbsPath{PathTool::Path2Join(parentPath, vidFileName)};
    const qint64 newVidSize = FileTool::GetFileSize(videoAbsPath);
    it = rawJsonDict.find(ENUM_2_STR(Size));
    if (it == rawJsonDict.cend()) {
      rawJsonDict.insert(ENUM_2_STR(Size), newVidSize);
      bJsonNeedUpdate = true;
    } else if (it.value().toLongLong() != newVidSize) {
      it->setValue(newVidSize);
      bJsonNeedUpdate = true;
    }
  }

  it = rawJsonDict.find(ENUM_2_STR(Rate));
  if (it == rawJsonDict.cend()) {
    rawJsonDict.insert(ENUM_2_STR(Rate), JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V);
    bJsonNeedUpdate = true;
  }

  return JsonOp::Counter{(int)bJsonNeedUpdate, (int)bJsonUsed, (int)bImgNameUpdate, (int)bVidNameUpdated};
}


JsonOp::Counter UpdateJsonKeyValuePair(const ScenesMixed& sMixed, JsonPr& jsonPr) {
  // will update m_Name, m_ImgName, m_VidName, m_Size
  bool bJsonNeedUpdate{false}, bJsonUsed{false}, bImgNameUpdate{false}, bVidNameUpdated{false};
  if (!jsonPr.isLocalFile()) {
    LOG_D("This json file is not we want");
    return JsonOp::Counter{(int)bJsonNeedUpdate, (int)bJsonUsed, (int)bImgNameUpdate, (int)bVidNameUpdated};
  }
  const QString& parentPath = jsonPr.GetJsonPrepath();
  const QString& jsonFileBaseName = PathTool::GetBaseName(jsonPr.GetJsonFileName());
  bJsonUsed = true;
  if (jsonPr.SyncNameValueFromFileBaseName()) {
    bJsonNeedUpdate = true;
  }

  QStringList imgsLst{sMixed.GetAllImgs(jsonFileBaseName)};
  if (jsonPr.m_ImgName != imgsLst) {
    jsonPr.m_ImgName.swap(imgsLst);
    bJsonNeedUpdate = true;
    bImgNameUpdate = true;
  }

  QString vidFileName{sMixed.GetFirstVid(jsonFileBaseName)};
  if (!vidFileName.isEmpty()) {
    if (jsonPr.m_VidName.isEmpty() || jsonPr.m_VidName != vidFileName) {
      jsonPr.m_VidName.swap(vidFileName);
      bJsonNeedUpdate = true;
      bVidNameUpdated = true;
    }
    const QString videoAbsPath{jsonPr.GetVideoAbsPath()};
    if (jsonPr.UpdateVideoSizeField(videoAbsPath)) {
      bJsonNeedUpdate = true;
    }
  }

  return JsonOp::Counter{(int)bJsonNeedUpdate, (int)bJsonUsed, (int)bImgNameUpdate, (int)bVidNameUpdated};
}

};