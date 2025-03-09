#include "SceneMixed.h"
#include "Tools/PathTool.h"
#include "Tools/ItemsPileCategory.h"
#include "PublicVariable.h"

using namespace ItemsPileCategory;

void ImagesNameSort(QStringList& imgs) {
  static const auto IMG_SORTER = [](const QString& lhs, const QString& rhs) -> bool { return lhs.size() < rhs.size() || (lhs.size() == rhs.size() && lhs < rhs); };
  std::sort(imgs.begin(), imgs.end(), IMG_SORTER);
}

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

QMap<QString, QStringList> ScenesMixed::operator()(const QStringList& files) {
  struct ImageName {
    QString baseName;
    QString extension;
  };
  QList<ImageName> imageNames;

  QMap<QString, QStringList> batches;

  for (const QString& medName : files) {
    QString baseName, ext;
    std::tie(baseName, ext) = PATHTOOL::GetBaseNameExt(medName);
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

  QRegularExpressionMatch result;
  for (const ImageName& imageName : imageNames) {
    QString imagefullname = imageName.baseName + imageName.extension;
    if (batches.contains(imageName.baseName)) {
      // a part 1.jpg
      // a part 1.json
      m_img2Name[imageName.baseName].append(imagefullname);
      batches[imageName.baseName].append(imagefullname);
      continue;
    }
    if ((result = IMG_PILE_NAME_PATTERN.match(imageName.baseName)).hasMatch()) {
      // ^(.*?)( | - )(\\d{1,3})?$
      // a.jpg
      // a 1.json
      // a - 1.json
      // a.json
      auto it = batches.constFind(result.captured(1));
      if (it != batches.constEnd()) {
        m_img2Name[it.key()].append(imagefullname);
      }
      batches[result.captured(1)].append(imagefullname);
      continue;
    }
    batches[imageName.baseName].append(imagefullname);
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
