#include "SceneInfoManager.h"
#include "PublicVariable.h"
#include "FileTool.h"
#include "SceneMixed.h"
#include "JsonHelper.h"
#include "PathTool.h"
#include "Logger.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QDateTime>

namespace SceneInfoManager {

QString ScnMgr::GetScnAbsFilePath(const QString& folderPath) {
  return folderPath + '/' + PathTool::fileName(folderPath) + ".scn";
}

Counter ScnMgr::UpdateJsonUnderAPath(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    return {};
  }
  int jsonUpdatedCnt = 0, jsonUsedCnt = 0;
  int imgNameKeyFieldUpdatedCnt = 0, vidNameKeyFieldUpdatedCnt = 0;

  ScenesMixed sMixed;
  sMixed(path);

  for (auto jsFileIt = sMixed.m_json2Name.cbegin(); jsFileIt != sMixed.m_json2Name.cend(); ++jsFileIt) {
    const QString& baseName = jsFileIt.key();
    const QString jPath = path + '/' + jsFileIt.value();
    QVariantHash rawJsonDict = JsonHelper::MovieJsonLoader(jPath);
    if (rawJsonDict.isEmpty()) {
      LOG_W("json file[%s] may corrupt read failed", qPrintable(jPath));
      continue;
    }

    QVariantHash::iterator it = rawJsonDict.find("Name");
    if (it == rawJsonDict.cend()) {
      LOG_D("This json file[%s] is not we want", qPrintable(jPath));
      continue;
    }

    bool jsonNeedUpdate{false};
    if (it.value() != baseName) {
      it->setValue(baseName);
      jsonNeedUpdate = true;
    }

    const QStringList& imgsLst = sMixed.GetAllImgs(baseName);
    it = rawJsonDict.find("ImgName");
    if (it == rawJsonDict.cend()) {
      rawJsonDict.insert("ImgName", imgsLst);
      jsonNeedUpdate = true;
      ++imgNameKeyFieldUpdatedCnt;
    } else if (it.value().toStringList() != imgsLst) {
      it->setValue(imgsLst);
      jsonNeedUpdate = true;
      ++imgNameKeyFieldUpdatedCnt;
    }

    const QString& vidFileName = sMixed.GetFirstVid(baseName);
    it = rawJsonDict.find("VidName");
    if (it == rawJsonDict.end()) {
      rawJsonDict.insert("VidName", vidFileName);
      jsonNeedUpdate = true;
      ++vidNameKeyFieldUpdatedCnt;
    } else if (it.value().toString() != vidFileName) {
      it->setValue(vidFileName);
      jsonNeedUpdate = true;
      ++vidNameKeyFieldUpdatedCnt;
    }

    if (!vidFileName.isEmpty()) {
      const QFileInfo vidFi{path + '/' + vidFileName};
      it = rawJsonDict.find("Uploaded");
      if (it == rawJsonDict.end()) {
#ifdef _WIN32
        rawJsonDict.insert("Uploaded", vidFi.birthTime().toString("yyyyMMdd hh:mm:ss"));
#else
        rawJsonDict.insert("Uploaded", vidFi.metadataChangeTime().toString("yyyyMMdd hh:mm:ss"));
#endif
        jsonNeedUpdate = true;
      } else if (it.value().toString().isEmpty()) {
#ifdef _WIN32
        it->setValue(vidFi.birthTime().toString("yyyyMMdd hh:mm:ss"));
#else
        it->setValue(vidFi.metadataChangeTime().toString("yyyyMMdd hh:mm:ss"));
#endif
        jsonNeedUpdate = true;
      }

      it = rawJsonDict.find("Size");
      qint64 newVidSize = vidFi.size();
      if (it == rawJsonDict.end()) {
        // construct Size
        rawJsonDict.insert("Size", newVidSize);
        jsonNeedUpdate = true;
      } else if (it.value().toLongLong() != newVidSize) {
        // first set Size
        it->setValue(vidFi.size());
        jsonNeedUpdate = true;
      }
    }

    if (rawJsonDict.find("Rate") == rawJsonDict.end()) {
      rawJsonDict.insert("Rate", 0);
      jsonNeedUpdate = true;
    }

    if (jsonNeedUpdate) {
      JsonHelper::DumpJsonDict(rawJsonDict, jPath);
      ++jsonUpdatedCnt;
    }
    ++jsonUsedCnt;
  }
  return Counter{jsonUpdatedCnt, jsonUsedCnt, vidNameKeyFieldUpdatedCnt, imgNameKeyFieldUpdatedCnt};
}

Counter ScnMgr::operator()(const QString& rootPath) {  // will iterate all sub
  if (!QFileInfo(rootPath).isDir()) {
    LOG_D("Not an existed directory[%s]", qPrintable(rootPath));
    return {};
  }
  Counter cnt{0, 0, 0, 0};
  QDirIterator folderIt{rootPath, {}, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
  while (folderIt.hasNext()) {
    cnt += UpdateJsonUnderAPath(folderIt.next());
  }
  cnt += UpdateJsonUnderAPath(rootPath);
  return cnt;
}

int ScnMgr::UpdateScnFiles(const QString& rootPath) {
  QDirIterator jsonIt{rootPath, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  QMap<QString, SceneInfoList> folder2Scenes;  // relativePathToJsonFile -> scenes
  int scnTotalCnt = 0;
  while (jsonIt.hasNext()) {
    QString jsonAbsPath = jsonIt.next();
    QVariantHash jsonHash = JsonHelper::MovieJsonLoader(jsonAbsPath);
    if (jsonHash.isEmpty()) {
      continue;
    }
    folder2Scenes[PathTool::absolutePath(jsonAbsPath)].push_back(SceneInfo::fromJsonVariantHash(jsonHash));
    ++scnTotalCnt;
  }

  if (scnTotalCnt == 0) {
    LOG_D("json dicts not exist at all under[%s], skip write", qPrintable(rootPath));
    return 0;
  }

  int scnFilesGeneratedCnt = 0;
  for (auto it = folder2Scenes.cbegin(); it != folder2Scenes.cend(); ++it) {
    const QString& scnAbsFolderPath = it.key();
    const SceneInfoList& scenes = it.value();

    LOG_D("%d scenes(s) under[%s] are found to generate a scn file", scenes.size(), qPrintable(scnAbsFolderPath));
    const QString& scnAbsFilePath{GetScnAbsFilePath(scnAbsFolderPath)};
    if (!SceneHelper::SaveScenesListToBinaryFile(scnAbsFilePath, scenes)) {
      continue;
    }
    ++scnFilesGeneratedCnt;
  }
  LOG_D("%d scn file(s) generated above", scnFilesGeneratedCnt);
  return scnFilesGeneratedCnt;
}

int ScnMgr::ClearScnFiles(const QString& rootPath) {
  QDirIterator folderIt{rootPath, {"*.scn"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  int scnTotalCnt = 0, deleteOkCnt = 0;
  while (folderIt.hasNext()) {
    deleteOkCnt += QFile::remove(folderIt.next());
    ++scnTotalCnt;
  }
  LOG_D("%d/%d scn file under[%s] delete succeed", deleteOkCnt, scnTotalCnt, qPrintable(rootPath));
  return deleteOkCnt;
}

}  // namespace SceneInfoManager
