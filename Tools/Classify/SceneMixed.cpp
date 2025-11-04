#include "SceneMixed.h"
#include "ItemsPileCategory.h"
#include "PathTool.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "JsonRenameRegex.h"
#include "StringTool.h"
#include "Logger.h"
#include <QDir>
#include <QSet>
using namespace ItemsPileCategory;

// Used for Floating Preview
ScenesMixed::GROUP_MAP_TYPE ScenesMixed::operator()(const QString& path) {
  QDir mediaDir(path, "", QDir::SortFlag::Name, QDir::Filter::Files);
  mediaDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET + TYPE_FILTER::IMAGE_TYPE_SET + TYPE_FILTER::JSON_TYPE_SET);
  return operator()(mediaDir.entryList());
}

bool ScenesMixed::NeedCombine2Folder(const QString& srcGrpName, const QString& dstGrpName) const {
  // dstGrpName: "movie", srcGrpName: "movie part 2" and without "movie part 2.json"
  // combine "movie part 2" into movie
  static const QSet<QString> ignoreDifferenceSet{
      "",                                                         //
      "part1",  "part2",   "part3",  "part4",  "part5",           //
      "partI",  "partII",                                         //
      "pt1",    "pt2",     "pt3",    "pt4",    "pt5",             //
      "scene1", "scene2",  "scene3", "scene4", "scene5",          //
      "sceneI", "sceneII",                                        //
      "sc1",    "sc2",     "sc3",    "sc4",    "sc5",             //
      "2160p",  "1080p",   "360p",   "480p",   "720p",   "810p",  //
      "4k",     "fhd",     "hd",     "sd",                        //
  };
  if (dstGrpName.size() >= srcGrpName.size()) {
    return false;
  }

  if (!srcGrpName.startsWith(dstGrpName)) {
    return false;
  }

  if (m_json2Name.contains(srcGrpName)) {
    return false;
  }
  QString differenceStr = srcGrpName.mid(dstGrpName.size()).toLower();
  differenceStr.replace(JSON_RENAME_REGEX::INVALID_GOOGLE_SEARCH_LETTER, "");
  return ignoreDifferenceSet.contains(differenceStr);
}

void Merge2ValuesInMapByKey(ScenesMixed::GROUP_MAP_TYPE& mp, const QString& srcKey, const QString& destKey) {
  auto srcIt = mp.find(srcKey);
  if (srcIt == mp.end()) {
    return;
  }
  mp[destKey] += std::move(srcIt.value());  // if no destKey in mp, then default construct mp[destKey] to QStringList{}
  mp.erase(srcIt);
}

struct stImg {
  QString baseName;
  QString fullName;
};

// Used for Name Ruler
ScenesMixed::GROUP_MAP_TYPE ScenesMixed::operator()(const QStringList& files) {
  QList<stImg> imageNames;

  GROUP_MAP_TYPE batches;

  static const T_DOT_EXT_2_TYPE& dotExt2TypeHash = GetTypeFromDotExtension();
  QString baseName, ext;
  for (const QString& medName : files) {
    std::tie(baseName, ext) = PathTool::GetBaseNameExt(medName);
    SCENE_COMPONENT_TYPE typeEnum = dotExt2TypeHash.value(ext.toLower(), SCENE_COMPONENT_TYPE::OTHER);
    switch (typeEnum) {
      case IMG: {
        imageNames.append({baseName, medName});
        break;
      }
      case VID: {
        m_vid2Name[baseName].append(medName);
        batches[baseName].append(medName);
        break;
      }
      case JSON: {
        m_json2Name[baseName] = medName;
        batches[baseName].append(medName);
        break;
      }
      default: {
        batches[baseName].append(medName);
        break;
      }
    }
  }

  // classify images into m_img2Name
  QRegularExpressionMatch matchResult;
  for (const stImg& img : imageNames) {
    QString imgGrpName{img.baseName};
    if (batches.contains(img.baseName)) {
      // Poster of a movie "Golden Island part 1.mp4" named "Golden Island part 1.jpg" should be in group "Golden Island part 1"
    } else if ((matchResult = IMG_PILE_NAME_PATTERN.match(img.baseName)).hasMatch()) {  // ^(.*?)( | - )(\\d{1,3})?$
      // Screenshot of a movie "Golden Island part 1.mp4" named "Golden Island part 1 0.jpg",  "Golden Island part 1 1.jpg" should be in group "Golden
      // Island part 1"
      imgGrpName = matchResult.captured(1);
    }
    // Image is neither a post nor a screenshot of a movie should be in a seperate group
    m_img2Name[imgGrpName].append(img.fullName);
    batches[imgGrpName].append(img.fullName);
  }

  if (batches.size() > 1) {
    auto it1 = batches.begin();  // dst
    auto it2 = it1 + 1;          // src
    while (it2 != batches.end()) {
      const QString& srcKey{it2.key()};
      const QString& dstKey{it1.key()};
      if (!NeedCombine2Folder(srcKey, dstKey)) {
        ++it1;
        ++it2;
        continue;
      }
      Merge2ValuesInMapByKey(m_img2Name, srcKey, dstKey);
      Merge2ValuesInMapByKey(m_vid2Name, srcKey, dstKey);
      it1.value() += std::move(it2.value());
      it2 = batches.erase(it2);
    }
  }

  for (QStringList& imgs : m_img2Name) {
    StringTool::ImgsSortNameLengthFirst(imgs);
  }

  LOG_D("%d items been grouped into %d groups, which are %d,%d,%d piles of img/vid/json repectively",  //
        files.size(), batches.size(), m_img2Name.size(), m_vid2Name.size(), m_json2Name.size());
  return batches;
}

const QStringList& ScenesMixed::GetAllImgs(const QString& baseName) const {
  auto it = m_img2Name.find(baseName);
  if (it == m_img2Name.cend()) {
    static QStringList tempEmpty;
    return tempEmpty;
  }
  return it.value();
}

const QString& ScenesMixed::GetFirstVid(const QString& baseName) const {
  auto it = m_vid2Name.find(baseName);
  if (it == m_vid2Name.cend()) {
    static QString vidNoExist;
    return vidNoExist;
  }
  return it.value().first();
}
