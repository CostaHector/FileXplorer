#include "SceneInfoManager.h"
#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"
#include "Tools/PathTool.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>

namespace SceneInfoManager {
SCENES_TYPE GetScenesFromPath(const QString& path, const bool enableFilter, const QString& pattern, SCENES_TYPE* pFiltered) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(path));
    return {};
  }
  const bool needFilter{enableFilter && pFiltered != nullptr && !pattern.isEmpty()};
  const int PATH_N = path.size();

  SCENES_TYPE scenes;
  QDirIterator jsonIt(path, {"*.scn"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (jsonIt.hasNext()) {
    const QString& scnFullPath{jsonIt.next()};
    const QString& rel2JsonFile = PATHTOOL::RelativePath2File(PATH_N, scnFullPath);
    scenes += ScnFileParser(scnFullPath, rel2JsonFile, needFilter, pattern, pFiltered);
  }
  if (pFiltered != nullptr && pattern.isEmpty()) {
    *pFiltered = scenes;
  }
  return scenes;
}

SCENES_TYPE& sort(SCENES_TYPE& scenes, SortByKey sortByKey, const bool reverse) {
  static const std::function<bool(const SCENE_INFO&, const SCENE_INFO&)> sortedFuncs[(int)SortByKey::BUTT] = {
      [&reverse](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool {
        return lhs.name < rhs.name ? (reverse ? false : true) : (reverse ? true : false);
      },
      [&reverse](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool {
        return lhs.vidSize < rhs.vidSize ? (reverse ? false : true) : (reverse ? true : false);
      },
      [&reverse](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool {
        return lhs.rate < rhs.rate ? (reverse ? false : true) : (reverse ? true : false);
      },
      [&reverse](const SCENE_INFO& lhs, const SCENE_INFO& rhs) -> bool {
        return lhs.uploaded < rhs.uploaded ? (reverse ? false : true) : (reverse ? true : false);
      }};
  if ((int)sortByKey >= (int)SortByKey::BUTT) {
    qWarning("key[%d] used to sort is out of bound[%d]", (int)sortByKey, (int)SortByKey::BUTT);
    return scenes;
  }
  std::sort(scenes.begin(), scenes.end(), sortedFuncs[(int)sortByKey]);
  return scenes;
}

int UpdateJsonImgVidSize(const QString& path) {  // will iterate all sub
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(path));
    return -1;
  }

  int errorJsonCnt = 0, jsonsCnt = 0, updatedCnt = 0, insertKeyCnt = 0, updatedValueCnt = 0;
  static const auto UpdateALevel = [&errorJsonCnt, &jsonsCnt, &updatedCnt, &insertKeyCnt, &updatedValueCnt](const QString& path) {
    QMap<QString, QString> img2Ext;  // images baseName, extension with prefix dot
    QMap<QString, QString> vid2FullPath;
    QDir mediaDir(path, "", QDir::SortFlag::Name, QDir::Filter::Files);
    mediaDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET + TYPE_FILTER::IMAGE_TYPE_SET);
    for (const QString& medName : mediaDir.entryList()) {
      QString name, ext;
      std::tie(name, ext) = PATHTOOL::GetBaseNameExt(medName);
      if (TYPE_FILTER::VIDEO_TYPE_SET.contains("*" + ext)) {
        vid2FullPath[name] = mediaDir.absoluteFilePath(medName);
      } else {
        img2Ext[name] = ext;
      }
    }

    QDir jsonDir(path, "", QDir::SortFlag::Name, QDir::Filter::Files);
    jsonDir.setNameFilters(TYPE_FILTER::JSON_TYPE_SET);
    for (const QString jsonName : jsonDir.entryList()) {
      const QString& jPath = jsonDir.absoluteFilePath(jsonName);
      QString jsonFileName, ext;
      std::tie(jsonFileName, ext) = PATHTOOL::GetBaseNameExt(jsonName);
      QVariantHash rawJsonDict = JsonFileHelper::MovieJsonLoader(jPath);
      if (rawJsonDict.isEmpty()) {
        ++errorJsonCnt;
        qWarning("json file[%s] read error", qPrintable(jPath));
        continue;
      }

      bool insertKey{false};
      bool updateValue{false};

      QVariantHash::iterator it{nullptr};
      const QString imgExt = img2Ext.value(jsonFileName, "");
      if (!imgExt.isEmpty()) {
        it = rawJsonDict.find("ImgName");
        const QString imgName = jsonFileName + imgExt;
        if (it == rawJsonDict.cend()) {
          rawJsonDict.insert("ImgName", imgName);
          insertKey = true;
        } else if (it.value().toString() != imgName) {
          it->setValue(imgName);
          updateValue = true;
        }
      }

      const QString vidPath = vid2FullPath.value(jsonFileName, "");
      if (!vidPath.isEmpty()) {
        QFileInfo vidFi{vidPath};
        it = rawJsonDict.find("VidName");
        if (it == rawJsonDict.end()) {
          rawJsonDict.insert("VidName", vidFi.fileName());
          insertKey = true;
        } else if (it.value().toString() != vidFi.fileName()) {
          it->setValue(vidFi.fileName());
          updateValue = true;
        }

        it = rawJsonDict.find("Uploaded");
        if (it == rawJsonDict.end()) {
          rawJsonDict.insert("Uploaded", vidFi.birthTime().toString("yyyyMMdd hh:mm:ss"));
          insertKey = true;
        } else if (it.value().toString().isEmpty()) {
          it->setValue(vidFi.birthTime().toString("yyyyMMdd hh:mm:ss"));
          updateValue = true;
        }

        it = rawJsonDict.find("VidSize");
        if (it == rawJsonDict.end()) {
          rawJsonDict.insert("VidSize", vidFi.size());
          insertKey = true;
        } else if (it.value().toLongLong() != vidFi.size()) {
          it->setValue(vidFi.size());
          updateValue = true;
        }
      }

      it = rawJsonDict.find("Rate");
      if (it == rawJsonDict.end()) {
        rawJsonDict.insert("Rate", 0);
        insertKey = true;
      }
      if (insertKey) {
        ++insertKeyCnt;
      }
      if (updateValue) {
        ++updatedValueCnt;
      }
      if (insertKey || updateValue) {
        JsonFileHelper::MovieJsonDumper(rawJsonDict, jPath);
        ++updatedCnt;
      }
      ++jsonsCnt;
    }
  };

  QDirIterator folderIt{path, {}, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
  while (folderIt.hasNext()) {
    UpdateALevel(folderIt.next());
  }
  UpdateALevel(path);
  qDebug("updated:%d, read fail:%d/total:%d; Insert Key:%d; Update Value:%d Json File(s)", updatedCnt, errorJsonCnt, jsonsCnt, insertKeyCnt,
         updatedValueCnt);
  return updatedCnt;
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
  QString vidSize;
  QString rate;
  bool isNum{false};
  while (!stream.atEnd()) {
    stream.readLineInto(&aScene.name, 256);
    stream.readLineInto(&aScene.imgName, 256);
    stream.readLineInto(&aScene.vidName, 256);
    stream.readLineInto(&vidSize, 20);
    stream.readLineInto(&rate, 5);
    aScene.vidSize = vidSize.toLongLong(&isNum);
    if (!isNum) {
      aScene.vidSize = 0;
    }
    isNum = false;
    aScene.rate = rate.toLongLong(&isNum);
    if (!isNum) {
      aScene.rate = 0;
    }
    isNum = false;
    stream.readLineInto(&aScene.uploaded, 32);

    scenesList.append(aScene);
    if (enableFilter && pFilterd != nullptr && aScene.name.contains(pattern, Qt::CaseSensitivity::CaseInsensitive)) {
      pFilterd->append(aScene);
    }
  }
  scnFi.close();
  qDebug("Read %d scenes out from %s succeed", scenesList.size(), qPrintable(scnFileFullPath));
  return scenesList;
}

int GenerateScnSameJsonLevel(const QString& jsonPath, QString& scnContent) {
  if (!QFileInfo(jsonPath).isDir()) {
    return 0;
  }
  QDir jsonDir{jsonPath, "", QDir::SortFlag::Name, QDir::Filter::Files};
  jsonDir.setNameFilters(TYPE_FILTER::JSON_TYPE_SET);
  int jsonFilesCnt = 0;
  for (const QString& jsonFileName : jsonDir.entryList()) {
    const QVariantHash rawJsonDict = JsonFileHelper::MovieJsonLoader(jsonDir.absoluteFilePath(jsonFileName));
    if (rawJsonDict.isEmpty()) {
      qDebug("Json file[%s] may corrupt.", qPrintable(jsonDir.absoluteFilePath(jsonFileName)));
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
    ++jsonFilesCnt;
  }
  qDebug("%d json(s) under[%s] are found to generate a scn file", jsonFilesCnt, qPrintable(jsonPath));
  return jsonFilesCnt;
}

bool GenerateAScnFile(const QString& aPath) {
  const QString folderFullPath{aPath};
  QString scnContent;
  int jsonUsedCnt = GenerateScnSameJsonLevel(folderFullPath, scnContent);
  if (jsonUsedCnt <= 0) {
    return false;
  }
  if (scnContent.isEmpty()) {
    return false;
  }

  QFile scnFi{folderFullPath + '/' + PATHTOOL::fileName(aPath) + ".scn"};
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

int GenerateScnFiles(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(path));
    return -1;
  }

  int scnFilesGeneratedCnt = 0;
  QDirIterator folderIt{path, {}, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
  while (folderIt.hasNext()) {
    if (GenerateAScnFile(folderIt.next())) {
      ++scnFilesGeneratedCnt;
    }
  }
  if (GenerateAScnFile(path)) {
    ++scnFilesGeneratedCnt;
  }
  qDebug("%d scn file(s) generated/updated under path[%s]", scnFilesGeneratedCnt, qPrintable(path));
  return scnFilesGeneratedCnt;
}

}  // namespace SceneInfoManager
