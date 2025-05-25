#include "SceneMixed.h"
#include "public/PathTool.h"
#include "Tools/ItemsPileCategory.h"
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

QMap<QString, QStringList> ScenesMixed::operator()(const QString& path) {
  QDir mediaDir(path, "", QDir::SortFlag::Name, QDir::Filter::Files);
  mediaDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET + TYPE_FILTER::IMAGE_TYPE_SET + TYPE_FILTER::JSON_TYPE_SET);
  return operator()(mediaDir.entryList());
}

bool ScenesMixed::NeedCombine2Folder(const QString& folderNameLhs, const QString& folderNameRhs) {
  // lhs: a
  // rhs: a part 2 contains not json file
  if (folderNameLhs.size() >= folderNameRhs.size()) {
    return false;
  }

  if (!folderNameRhs.startsWith(folderNameLhs)) {
    return false;
  }

  if (m_json2Name.contains(folderNameRhs)) {
    return false;
  }

  return true;
}

QMap<QString, QStringList> ScenesMixed::operator()(const QStringList& files) {
  struct ImageName {
    QString baseName;
    QString extension;
  };
  QList<ImageName> imageNames;

  QMap<QString, QStringList> batches;

  for (const QString& medName : files) {
    QString baseName, ext;
    std::tie(baseName, ext) = PathTool::GetBaseNameExt(medName);
    SCENE_COMPONENT_TYPE typeEnum = DOT_EXT_2_TYPE.value(ext.toLower(), SCENE_COMPONENT_TYPE::OTHER);
    switch (typeEnum) {
      case IMG: {
        imageNames.append({baseName, ext});
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
  QRegularExpressionMatch result;
  for (const ImageName& imageName : imageNames) {
    QString imagefullname = imageName.baseName + imageName.extension;
    if (batches.contains(imageName.baseName)) {
      // a part 1.jpg => folder name "a part 1"
      m_img2Name[imageName.baseName].append(imagefullname);
      batches[imageName.baseName].append(imagefullname);
      continue;
    }
    if ((result = IMG_PILE_NAME_PATTERN.match(imageName.baseName)).hasMatch()) {
      // ^(.*?)( | - )(\\d{1,3})?$
      // a.jpg => folder name "a"
      // a 1.json
      // a - 1.json
      // a.json
      m_img2Name[result.captured(1)].append(imagefullname);
      batches[result.captured(1)].append(imagefullname);
      continue;
    }
    batches[imageName.baseName].append(imagefullname);
  }

  if (batches.size() > 1) {
    auto it1 = batches.begin();
    auto it2 = it1 + 1;
    while (it1 != it2 && it1 != batches.end() && it2 != batches.end()) {
      // combine items in it2 into it1
      const QString& it1FolderName = it1.key();
      const QString& it2FolderName = it2.key();
      bool needCombine = NeedCombine2Folder(it1FolderName, it2FolderName);
      if (!needCombine) {
        ++it1;
        ++it2;
        continue;
      }

      auto tmpImgIt = m_img2Name.find(it2FolderName);
      if (tmpImgIt != m_img2Name.end()) {
        m_img2Name[it1FolderName] += tmpImgIt.value();
        m_img2Name.erase(tmpImgIt);
      }
      auto tmpVidIt = m_vid2Name.find(it2FolderName);
      if (tmpVidIt != m_vid2Name.end()) {
        m_vid2Name[it1FolderName] += tmpVidIt.value();
        m_vid2Name.erase(tmpVidIt);
      }

      it1.value() += it2.value();
      it2 = batches.erase(it2);
    }
  }

  for (QStringList& imgs : m_img2Name) {
    ImagesNameSort(imgs);
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
