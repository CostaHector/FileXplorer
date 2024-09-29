#include "SceneInfoManager.h"
#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>

SCENES_TYPE SceneInfoManager::GetScenesFromPath(const QString& path, const bool enableFilter, const QString& pattern, SCENES_TYPE* pFiltered) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(path));
    return {};
  }
  const bool needFilter{enableFilter && pFiltered != nullptr && !pattern.isEmpty()};

  SCENES_TYPE scenes;
  QDirIterator jsonIt(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (jsonIt.hasNext()) {
    const QFileInfo jFi{jsonIt.next()};
    const QVariantHash rawJsonDict = JsonFileHelper::MovieJsonLoader(jFi.absoluteFilePath());
    scenes.append(SCENE_INFO{rawJsonDict.value("Name", "").toString(), rawJsonDict.value("ImgName", "").toString(),
                             rawJsonDict.value("VidName", "").toString(), rawJsonDict.value("VidSize", 0).toLongLong(),
                             rawJsonDict.value("Rate", 0).toInt(), rawJsonDict.value("Uploaded", "").toString()});
    if (needFilter && jFi.fileName().contains(pattern, Qt::CaseSensitivity::CaseInsensitive)) {
      pFiltered->append(scenes.back());
    }
  }
  if (pFiltered != nullptr && pattern.isEmpty()) {
    *pFiltered = scenes;
  }
  return scenes;
}

void SceneInfoManager::sort(const SortByKey sortByKey, const bool reverse) {
  // std::less<QString>(), std::greater<QString>()
  std::function<bool(const SCENE_INFO&, const SCENE_INFO&)> sortedFuncs[5] = {
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
  std::sort(mScenes.begin(), mScenes.end(), sortedFuncs[sortByKey]);
}

int SceneInfoManager::UpdateJsonImgVidSize(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not a directory", qPrintable(path));
    return -1;
  }

  static auto getNameAndExt = [](const QString& fullpath) -> std::pair<QString, QString> {
    int lastIndexOfSlash = fullpath.lastIndexOf('/');
    int lastIndexOfExtDot = fullpath.lastIndexOf('.');
    if (lastIndexOfSlash != -1 && lastIndexOfSlash < lastIndexOfExtDot) {
      return std::make_pair(fullpath.mid(lastIndexOfSlash + 1, lastIndexOfExtDot - lastIndexOfSlash - 1), fullpath.mid(lastIndexOfExtDot + 1));
    }
    return {};
  };

  QMap<QString, QString> img2Ext;
  QMap<QString, QString> vid2FullPath;
  QDirIterator mediaIt(path, TYPE_FILTER::VIDEO_TYPE_SET + TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files,
                       QDirIterator::IteratorFlag::Subdirectories);
  while (mediaIt.hasNext()) {
    const QString& medPath = mediaIt.next();
    QString name, ext;
    std::tie(name, ext) = getNameAndExt(medPath);
    if (TYPE_FILTER::VIDEO_TYPE_SET.contains("*." + ext)) {
      vid2FullPath[name] = medPath;
    } else {
      img2Ext[name] = ext;
    }
  }

  int errorJsonCnt = 0, jsonsCnt = 0, updatedCnt = 0, insertKeyCnt = 0, updatedValueCnt = 0;
  QDirIterator jsonIt(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (jsonIt.hasNext()) {
    const QString& jPath = jsonIt.next();
    QString jsonFileName, ext;
    std::tie(jsonFileName, ext) = getNameAndExt(jPath);
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
      const QString imgName = jsonFileName + '.' + imgExt;
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
  qDebug("updated:%d, read fail:%d/total:%d; Insert Key:%d; Update Value:%d Json File(s)", updatedCnt, errorJsonCnt, jsonsCnt, insertKeyCnt,
         updatedValueCnt);
  return updatedCnt;
}
