#include "SceneInfoManager.h"
#include "PublicVariable.h"
#include "Tools/Classify/SceneMixed.h"
#include "Tools/JsonFileHelper.h"
#include "Tools/PathTool.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>

QString SCENE_INFO::GetFirstKImagesLabel(const QString& rootPath, const int k) const {
  static const QString IMAGE_LABEL_TEMPLATE = R"(<img alt="%1" height="480" src="%1"/>)";
  QString imgLabels;
  for (int i = 0; i < k && i < imgs.size(); ++i) {
    imgLabels += IMAGE_LABEL_TEMPLATE.arg(rootPath + rel2scn + imgs[i]);
  }
  return imgLabels;
}

namespace SceneInfoManager {
SceneSortOption GetSortOptionFromStr(const QString& sortOption) {
  static const QMap<QString, SceneSortOption> option2SortEnum{
      {"Movie Name", SceneSortOption::NAME}, {"Movie Size", SceneSortOption::SIZE}, {"Rate", SceneSortOption::RATE}, {"Uploaded Time", SceneSortOption::UPLOADED}};
  return option2SortEnum.value(sortOption, SceneSortOption::BUTT);
}

SCENES_TYPE GetScenesFromPath(const QString& path, const bool enableFilter, const QString& pattern, SCENES_TYPE* pScnFiltered) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(path));
    return {};
  }
  const bool needFilter{enableFilter && pScnFiltered != nullptr && !pattern.isEmpty()};
  const int PATH_N = path.size();

  SCENES_TYPE scnTotals;
  int scnFilesCnt = 0, scnFilteredCnt = -1;
  QDirIterator jsonIt(path, {"*.scn"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (jsonIt.hasNext()) {
    const QString& scnFullPath{jsonIt.next()};
    const QString& rel2JsonFile = PATHTOOL::RelativePath2File(PATH_N, scnFullPath);
    scnTotals += ScnFileParser(scnFullPath, rel2JsonFile, needFilter, pattern, pScnFiltered);
    ++scnFilesCnt;
  }
  if (enableFilter && pScnFiltered != nullptr && pattern.isEmpty()) {
    *pScnFiltered = scnTotals;
  }
  if (pScnFiltered != nullptr) {
    scnFilteredCnt = pScnFiltered->size();
  }
  qDebug("%d total, %d filtered scenes get from %d *.scn file(s)", scnTotals.size(), scnFilteredCnt, scnFilesCnt);
  return scnTotals;
}

SCENES_TYPE& sort(SCENES_TYPE& scenes, SceneSortOption sortByKey, const bool reverse) {
  if ((char)sortByKey >= (int)SceneSortOption::BUTT) {
    qWarning("key[%d] used to sort is out of bound[%d]", (char)sortByKey, (char)SceneSortOption::BUTT);
    return scenes;
  }
  static const std::function<bool(const SCENE_INFO&, const SCENE_INFO&)> sortedMap[2][(int)SceneSortOption::BUTT] = {
      // < operator here
      {[](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.name < rhs.name; }, [](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.vidSize < rhs.vidSize; },
       [](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.rate < rhs.rate; }, [](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.uploaded < rhs.uploaded; }},
      // > operator here
      {[](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.name > rhs.name; }, [](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.vidSize > rhs.vidSize; },
       [](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.rate > rhs.rate; }, [](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool { return lhs.uploaded > rhs.uploaded; }}};
  std::sort(scenes.begin(), scenes.end(), sortedMap[(int)reverse][(int)sortByKey]);
  return scenes;
}

SCENES_TYPE ScnFileParser(const QString& scnFileFullPath, const QString rel, bool enableFilter, const QString& pattern, SCENES_TYPE* pFilterd) {
  QFile scnFi{scnFileFullPath};
  if (!scnFi.exists()) {
    qDebug("scn file[%s] not exist", qPrintable(scnFileFullPath));
    return {};
  }

  if (!scnFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qCritical("Open scn file[%s] to read failed", qPrintable(scnFi.fileName()));
    return {};
  }
  QTextStream stream(&scnFi);
  stream.setCodec("UTF-8");

  SCENES_TYPE scenesList;

  SCENE_INFO aScene;
  aScene.rel2scn = rel;

  while (!stream.atEnd()) {
    // name
    if (!stream.readLineInto(&aScene.name, 256)) {
      qWarning("read name line failed");
      return {};
    }
    // images list: a.jpg|a 2.png|a 4.webp
    QString imgsSeperatedByVerticalBar;
    if (!stream.readLineInto(&imgsSeperatedByVerticalBar)) {
      qWarning("read imgs line failed");
      return {};
    }
    aScene.imgs = imgsSeperatedByVerticalBar.split('|');
    // Video Name: video.mp4
    if (!stream.readLineInto(&aScene.vidName, 256)) {
      qWarning("read video name line failed");
      return {};
    }
    // Video Size: 10240kByte
    QString vidSizeStr;
    if (!stream.readLineInto(&vidSizeStr, 20)) {
      qWarning("read video size line failed");
      return {};
    }
    bool isVidSizeStrNum{false};
    aScene.vidSize = vidSizeStr.toLongLong(&isVidSizeStrNum);
    if (!isVidSizeStrNum) {
      qWarning("Video size string[%s] is not a number.", qPrintable(vidSizeStr));
      aScene.vidSize = 0;
      return {};
    }
    // Rate: 10'
    QString rateStr;
    if (!stream.readLineInto(&rateStr, 5)) {
      qWarning("read rate line failed");
      return {};
    }
    bool isRateStrNum = false;
    aScene.rate = rateStr.toLongLong(&isRateStrNum);
    if (!isRateStrNum) {
      qWarning("Rate string[%s] is not a number.", qPrintable(rateStr));
      aScene.rate = 0;
      return {};
    }
    // uploaded time: 2024/12/12 12:50:50
    if (!stream.readLineInto(&aScene.uploaded, 32)) {
      qWarning("Uploaded time read failed.");
      return {};
    }

    scenesList.append(aScene);
    if (enableFilter && aScene.name.contains(pattern, Qt::CaseSensitivity::CaseInsensitive)) {
      pFilterd->append(aScene);
    }
  }
  scnFi.close();
  qDebug("Read %d scenes out from file[%s] succeed", scenesList.size(), qPrintable(scnFileFullPath));
  return scenesList;
}

std::pair<QString, int> GetScnFileContents(const QStringList& jsonNames, const QList<QVariantHash>& jsonDicts) {
  if (jsonNames.size() != jsonDicts.size()) {
    qWarning("json file name count:%d, dict count:%d", jsonNames.size(), jsonDicts.size());
    return std::pair<QString, int>{"", -1};
  }
  int jsonUsedCnt = 0;
  QString scnContent;
  for (int i = 0; i < jsonNames.size(); ++i) {
    const QVariantHash& rawJsonDict = jsonDicts[i];
    const QString& jsonFileName = jsonNames[i];
    if (rawJsonDict.isEmpty()) {
      qDebug("Json dict[%s] may corrupt.", qPrintable(jsonFileName));
      continue;
    }
    scnContent += rawJsonDict.value("Name", "").toString();
    scnContent += '\n';
    scnContent += rawJsonDict.value("ImgName", "").toString();
    scnContent += '\n';
    scnContent += rawJsonDict.value("VidName", "").toString();
    scnContent += '\n';
    scnContent += QString::number(rawJsonDict.value("VidSize", 0).toULongLong());
    scnContent += '\n';
    scnContent += QString::number(rawJsonDict.value("Rate", 0).toInt());
    scnContent += '\n';
    scnContent += rawJsonDict.value("Uploaded", "").toString();
    scnContent += '\n';
    ++jsonUsedCnt;
  }
  return std::pair<QString, int>(scnContent, jsonUsedCnt);
}

bool GenerateAScnFile(const QString& aPath) {
  if (!QFileInfo(aPath).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(aPath));
    return false;
  }

  QDir jsonDir{aPath, {}, QDir::SortFlag::Name, QDir::Filter::Files};
  jsonDir.setNameFilters(TYPE_FILTER::JSON_TYPE_SET);
  const QStringList& jsonNames = jsonDir.entryList();
  QList<QVariantHash> jsonDicts;
  jsonDicts.reserve(jsonNames.size());
  for (const QString& jsonFileName : jsonNames) {
    jsonDicts << JsonFileHelper::MovieJsonLoader(jsonDir.absoluteFilePath(jsonFileName));
  }
  int jsonUsedCnt = 0;
  QString scnContent;
  std::tie(scnContent, jsonUsedCnt) = GetScnFileContents(jsonNames, jsonDicts);
  qDebug("%d json(s) under[%s] are found to generate a scn file", jsonUsedCnt, qPrintable(aPath));

  if (jsonUsedCnt == 0 || scnContent.isEmpty()) {
    return false;
  }

  QFile scnFi{aPath + '/' + PATHTOOL::fileName(aPath) + ".scn"};
  if (!scnFi.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qCritical("Open scn file[%s] to write failed.", qPrintable(scnFi.fileName()));
    return false;
  }

  QTextStream stream(&scnFi);
  stream.setCodec("UTF-8");
  stream << scnContent;
  scnFi.close();
  return true;
}

int GenerateScnFilesDirectly(const QString& rootPath) {
  if (!QFileInfo(rootPath).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(rootPath));
    return -1;
  }

  int scnFilesGeneratedCnt = 0;
  QDirIterator folderIt{rootPath, {}, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
  while (folderIt.hasNext()) {
    if (GenerateAScnFile(folderIt.next())) {
      ++scnFilesGeneratedCnt;
    }
  }
  if (GenerateAScnFile(rootPath)) {
    ++scnFilesGeneratedCnt;
  }
  qDebug("%d scn file(s) generated/updated under path[%s]", scnFilesGeneratedCnt, qPrintable(rootPath));
  return scnFilesGeneratedCnt;
}

}  // namespace SceneInfoManager

int JsonDataRefresher::UpdateAFolderItself(const QString& path) {
  int updatedJsonFilesCnt = 0, usefullJsonCnt = 0;

  ScenesMixed sMixed;
  sMixed(path);

  for (auto jsFileIt = sMixed.m_json2Name.cbegin(); jsFileIt != sMixed.m_json2Name.cend(); ++jsFileIt) {
    const QString& baseName = jsFileIt.key();
    const QString jPath = path + '/' + jsFileIt.value();
    QVariantHash rawJsonDict = JsonFileHelper::MovieJsonLoader(jPath);
    if (rawJsonDict.isEmpty()) {
      qWarning("json file[%s] may corrupt read failed", qPrintable(jPath));
      continue;
    }

    QVariantHash::iterator it = rawJsonDict.find("Name");
    if (it == rawJsonDict.cend()) {
      qDebug("This json file[%s] is not we want", qPrintable(jPath));
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
    } else if (it.value().toStringList() != imgsLst) {
      it->setValue(imgsLst);
      jsonNeedUpdate = true;
    }

    const QString& vidFileName = sMixed.GetFirstVid(baseName);
    it = rawJsonDict.find("VidName");
    if (it == rawJsonDict.end()) {
      rawJsonDict.insert("VidName", vidFileName);
      jsonNeedUpdate = true;
    } else if (it.value().toString() != vidFileName) {
      it->setValue(vidFileName);
      jsonNeedUpdate = true;
    }

    if (!vidFileName.isEmpty()) {
      const QFileInfo vidFi{path + '/' + vidFileName};
      it = rawJsonDict.find("Uploaded");
      if (it == rawJsonDict.end()) {
        // construct birth Time
        rawJsonDict.insert("Uploaded", vidFi.birthTime().toString("yyyyMMdd hh:mm:ss"));
        jsonNeedUpdate = true;
      } else if (it.value().toString().isEmpty()) {
        // first set change birth Time
        it->setValue(vidFi.birthTime().toString("yyyyMMdd hh:mm:ss"));
        jsonNeedUpdate = true;
      }

      it = rawJsonDict.find("VidSize");
      if (it == rawJsonDict.end()) {
        // construct VidSize
        rawJsonDict.insert("VidSize", vidFi.size());
        jsonNeedUpdate = true;
      } else if (it.value().toLongLong() != 0) {
        // first set VidSize
        it->setValue(vidFi.size());
        jsonNeedUpdate = true;
      }
    }

    if (rawJsonDict.find("Rate") == rawJsonDict.end()) {
      rawJsonDict.insert("Rate", 0);
      jsonNeedUpdate = true;
    }

    if (jsonNeedUpdate) {
      JsonFileHelper::MovieJsonDumper(rawJsonDict, jPath);
      ++updatedJsonFilesCnt;
    }
    ++usefullJsonCnt;

    m_jsonsDicts[path].append(rawJsonDict);
  }
  m_updatedJsonFilesCnt += updatedJsonFilesCnt;
  m_usefullJsonCnt += usefullJsonCnt;
  return updatedJsonFilesCnt;
};

int JsonDataRefresher::operator()(const QString& rootPath) {  // will iterate all sub
  if (!QFileInfo(rootPath).isDir()) {
    qDebug("Not an existed directory[%s]", qPrintable(rootPath));
    return -1;
  }
  QDirIterator folderIt{rootPath, {}, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
  while (folderIt.hasNext()) {
    UpdateAFolderItself(folderIt.next());
  }
  UpdateAFolderItself(rootPath);
  qDebug("%d useful json file(s) founded, %d json file(s) updated from %d folder(s) In path[%s]", m_usefullJsonCnt, m_updatedJsonFilesCnt, m_jsonsDicts.size(), qPrintable(rootPath));
  return m_updatedJsonFilesCnt;
}

int JsonDataRefresher::GenerateScnFiles() {
  int scnFilesGeneratedCnt = 0;
  for (auto it = m_jsonsDicts.cbegin(); it != m_jsonsDicts.cend(); ++it) {
    const QString& path2Jsons = it.key();
    const QList<QVariantHash>& dicts = it.value();
    int usefullJsonCnt = 0;
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
      scnContent += QString::number(rawJsonDict.value("VidSize", 0).toULongLong());
      scnContent += '\n';
      scnContent += QString::number(rawJsonDict.value("Rate", 0).toInt());
      scnContent += '\n';
      scnContent += rawJsonDict.value("Uploaded", "").toString();
      scnContent += '\n';
      ++usefullJsonCnt;
    }
    qDebug("%d json(s) under[%s] are found to generate a scn file", usefullJsonCnt, qPrintable(path2Jsons));
    if (usefullJsonCnt <= 0) {
      return false;
    }
    if (scnContent.isEmpty()) {
      return false;
    }
    QFile scnFi{path2Jsons + '/' + PATHTOOL::fileName(path2Jsons) + ".scn"};
    if (!scnFi.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qCritical("Open scn file[%s] to write failed.", qPrintable(scnFi.fileName()));
      return false;
    }
    QTextStream stream(&scnFi);
    stream.setCodec("UTF-8");
    stream << scnContent;
    scnFi.close();
    ++scnFilesGeneratedCnt;
  }
  qDebug("%d scn file(s) generated above", scnFilesGeneratedCnt);
  return scnFilesGeneratedCnt;
}
