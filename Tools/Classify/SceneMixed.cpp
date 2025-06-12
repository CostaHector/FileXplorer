#include "SceneMixed.h"
#include "ItemsPileCategory.h"
#include "public/PathTool.h"
#include "public/PublicTool.h"
#include "public/PublicVariable.h"

using namespace ItemsPileCategory;

void SetElementIndexFirstIfValueFirst(QStringList& lst) {
  if (lst.size() < 2) {
    return;
  }
  auto minIt = std::min_element(lst.begin(), lst.end(), [](const QString& l, const QString& r) -> bool { return l.size() < r.size() || (l.size() == r.size() && l < r); });
  if (minIt != lst.begin()) {
    lst.front().swap(*minIt);
  }
}
// Used for Floating Preview
QMap<QString, QStringList> ScenesMixed::operator()(const QString& path) {
  QDir mediaDir(path, "", QDir::SortFlag::Name, QDir::Filter::Files);
  mediaDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET + TYPE_FILTER::IMAGE_TYPE_SET + TYPE_FILTER::JSON_TYPE_SET);
  return operator()(mediaDir.entryList());
}

bool ScenesMixed::NeedCombine2Folder(const QString& srcGrpName, const QString& dstGrpName) const {
  // srcGrpName: movie
  // dstGrpName: movie part 2. and contains no json file
  if (dstGrpName.size() >= srcGrpName.size()) {
    return false;
  }

  if (!srcGrpName.startsWith(dstGrpName)) {
    return false;
  }

  if (m_json2Name.contains(srcGrpName)) {
    return false;
  }

  return true;
}
// Used for Name Ruler
QMap<QString, QStringList> ScenesMixed::operator()(const QStringList& files) {
  struct stImg {
    QString baseName;
    QString fullName;
  };
  QList<stImg> imageNames;

  QMap<QString, QStringList> batches;

  for (const QString& medName : files) {
    QString baseName, ext;
    std::tie(baseName, ext) = PathTool::GetBaseNameExt(medName);
    SCENE_COMPONENT_TYPE typeEnum = DOT_EXT_2_TYPE.value(ext.toLower(), SCENE_COMPONENT_TYPE::OTHER);
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
      // Screenshot of a movie "Golden Island part 1.mp4" named "Golden Island part 1 0.jpg",  "Golden Island part 1 1.jpg" should be in group "Golden Island part 1"
      imgGrpName = matchResult.captured(1);
    }
    // Image is neither a post nor a screenshot of a movie should be in a seperate group
    m_img2Name[imgGrpName].append(img.fullName);
    batches[imgGrpName].append(img.fullName);
  }

  if (batches.size() > 1) {
    static const auto merge2ValuesInMapByKey = [](decltype(batches)& mp, const QString& srcKey, const QString& destKey) {
      auto srcIt = mp.find(srcKey);
      if (srcIt == mp.end()) {
        return;
      }
      mp[destKey] += std::move(srcIt.value()); // if no destKey in mp, then default construct mp[destKey] to QStringList{}
      mp.erase(srcIt);
    };

    auto it1 = batches.begin(); // dst
    auto it2 = it1 + 1; // src
    while (it2 != batches.end()) {
      const QString& srcKey{it2.key()};
      const QString& dstKey{it1.key()};
      if (!NeedCombine2Folder(srcKey, dstKey)) {
        ++it1;
        ++it2;
        continue;
      }
      merge2ValuesInMapByKey(m_img2Name, srcKey, dstKey);
      merge2ValuesInMapByKey(m_vid2Name, srcKey, dstKey);
      it1.value() += std::move(it2.value());
      it2 = batches.erase(it2);
    }
  }

  for (QStringList& imgs : m_img2Name) {
    ImgsSortNameLengthFirst(imgs);
  }

  qDebug("%d,%d,%d piles of img/vid/json found from %d item(s) given", m_img2Name.size(), m_vid2Name.size(), m_json2Name.size(), files.size());
  return batches;
}

const QString& ScenesMixed::GetFirstImg(const QString& baseName) const {
  auto it = m_img2Name.find(baseName);
  if (it == m_img2Name.cend()) {
    static QString imgNoExist;
    return imgNoExist;
  }
  return it.value().first();
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
