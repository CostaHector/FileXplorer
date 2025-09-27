#include "SceneInfoManager.h"
#include "PublicVariable.h"
#include "PublicTool.h"
#include "SceneMixed.h"
#include "JsonHelper.h"
#include "PathTool.h"
#include "Logger.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QTextStream>
#include <QDateTime>

namespace SceneInfoManager {
SCENE_INFO_LIST GetScnsLstFromPath(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    LOG_D("path[%s] is not a directory", qPrintable(path));
    return {};
  }
  const int PATH_N = path.size();

  SCENE_INFO_LIST scnTotals;
  int scnFilesCnt = 0;
  QDirIterator jsonIt(path, {"*.scn"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (jsonIt.hasNext()) {
    const QString& scnFullPath{jsonIt.next()};
    const QString& rel2JsonFile = PathTool::RelativePath2File(PATH_N, scnFullPath);
    scnTotals += ParseAScnFile(scnFullPath, rel2JsonFile);
    ++scnFilesCnt;
  }
  std::sort(scnTotals.begin(), scnTotals.end());
  LOG_D("total %d scenes get from %d *.scn file(s)", scnTotals.size(), scnFilesCnt);
  return scnTotals;
}

SCENE_INFO_LIST ParseAScnFile(const QString& scnFileFullPath, const QString rel) {
  QFile scnFi{scnFileFullPath};
  if (!scnFi.exists()) {
    LOG_D("scn file[%s] not exist", qPrintable(scnFileFullPath));
    return {};
  }
  if (!scnFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_C("Open scn file[%s] to read failed", qPrintable(scnFi.fileName()));
    return {};
  }

  QTextStream stream(&scnFi);
  stream.setCodec("UTF-8");

  SCENE_INFO_LIST scenesList;

  SCENE_INFO aScene;
  aScene.rel2scn = rel;

  while (!stream.atEnd()) {
    // name
    if (!stream.readLineInto(&aScene.name, 256)) {
      LOG_W("read name line failed");
      return {};
    }
    // images list: a.jpg|a 2.png|a 4.webp
    QString imgsSeperatedByVerticalBar;
    if (!stream.readLineInto(&imgsSeperatedByVerticalBar)) {
      LOG_W("read imgs line failed");
      return {};
    }
    aScene.imgs.clear();
    if (!imgsSeperatedByVerticalBar.isEmpty()) {
      aScene.imgs = imgsSeperatedByVerticalBar.split('|');
    }
    // Video Name: video.mp4
    if (!stream.readLineInto(&aScene.vidName, 256)) {
      LOG_W("read video name line failed");
      return {};
    }
    // Video Size: 10240kByte
    QString vidSizeStr;
    if (!stream.readLineInto(&vidSizeStr, 20)) {
      LOG_W("read video size line failed");
      return {};
    }
    bool isVidSizeStrNum{false};
    aScene.vidSize = vidSizeStr.toLongLong(&isVidSizeStrNum);
    if (!isVidSizeStrNum) {
      LOG_W("Video size string[%s] is not a number.", qPrintable(vidSizeStr));
      aScene.vidSize = 0;
      return {};
    }
    // Rate: 10'
    QString rateStr;
    if (!stream.readLineInto(&rateStr, 5)) {
      LOG_W("read rate line failed");
      return {};
    }
    bool isRateStrNum = false;
    aScene.rate = rateStr.toLongLong(&isRateStrNum);
    if (!isRateStrNum) {
      LOG_W("Rate string[%s] is not a number.", qPrintable(rateStr));
      aScene.rate = 0;
      return {};
    }
    // uploaded time: 2024/12/12 12:50:50
    if (!stream.readLineInto(&aScene.uploaded, 32)) {
      LOG_W("Uploaded time read failed.");
      return {};
    }

    scenesList.append(aScene);
  }
  scnFi.close();
  LOG_D("Read %d scenes out from file[%s] succeed", scenesList.size(), qPrintable(scnFileFullPath));
  return scenesList;
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

    m_jsonsDicts[path].append(rawJsonDict);
  }
  return {jsonUpdatedCnt, jsonUsedCnt, imgNameKeyFieldUpdatedCnt, vidNameKeyFieldUpdatedCnt};
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
  LOG_D("%d useful json file(s) founded and %d get updated(imgUpdate:%d, vidUpdate:%d) In path[%s]",  //
        cnt.m_jsonUsedCnt, cnt.m_jsonUpdatedCnt,                                                      //
        cnt.m_ImgNameKeyFieldUpdatedCnt, cnt.m_VidNameKeyFieldUpdatedCnt,                             //
        qPrintable(rootPath));                                                                        //
  return cnt;
}

int ScnMgr::WriteDictIntoScnFiles() {
  if (m_jsonsDicts.isEmpty()) {
    LOG_D("json dicts not exist, skip write");
    return 0;
  }
  int scnFilesGeneratedCnt = 0;
  for (auto it = m_jsonsDicts.cbegin(); it != m_jsonsDicts.cend(); ++it) {
    const QString& path2Jsons = it.key();
    const QList<QVariantHash>& dicts = it.value();

    int jsonUsedCnt = 0;
    QString scnContent;
    for (const QVariantHash& rawJsonDict : dicts) {
      if (rawJsonDict.isEmpty()) {
        continue;
      }
      scnContent += rawJsonDict.value("Name", "").toString();
      scnContent += '\n';
      scnContent += rawJsonDict.value("ImgName", "").toStringList().join('|');
      scnContent += '\n';
      scnContent += rawJsonDict.value("VidName", "").toString();
      scnContent += '\n';
      scnContent += QString::number(rawJsonDict.value("Size", 0).toULongLong());
      scnContent += '\n';
      scnContent += QString::number(rawJsonDict.value("Rate", 0).toInt());
      scnContent += '\n';
      scnContent += rawJsonDict.value("Uploaded", "").toString();
      scnContent += '\n';
      ++jsonUsedCnt;
    }
    LOG_D("%d json(s) under[%s] are found to generate a scn file", jsonUsedCnt, qPrintable(path2Jsons));
    if (jsonUsedCnt == 0) {
      continue;
    }
    const QString scnAbsFilePath = path2Jsons + '/' + PathTool::fileName(path2Jsons) + ".scn";
    if (!FileTool::TextWriter(scnAbsFilePath, scnContent, QIODevice::WriteOnly | QIODevice::Text)) {
      LOG_W("Write %d char(s) contents into file[%s] failed", scnContent.size(), qPrintable(scnAbsFilePath));
      return scnFilesGeneratedCnt;
    }
    ++scnFilesGeneratedCnt;
  }
  m_jsonsDicts.clear();
  LOG_D("%d scn file(s) generated above", scnFilesGeneratedCnt);
  return scnFilesGeneratedCnt;
}

}  // namespace SceneInfoManager
