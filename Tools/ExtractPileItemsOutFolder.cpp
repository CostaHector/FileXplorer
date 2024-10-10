#include "ExtractPileItemsOutFolder.h"
#include "Tools/PathTool.h"
#include <QRegularExpression>
#include <QDir>

enum SCENE_COMPONENT_TYPE : char { IMG = 0, VID = 1, JSON = 2, OTHER = 3 };

QMap<QString, SCENE_COMPONENT_TYPE> DOT_EXT_2_TYPE{{".jpg", IMG}, {".jpeg", IMG}, {".png", IMG}, {".webp", IMG}, {".jfif", IMG}, {".webp", IMG},
                                                   {".mp4", VID}, {".mkv", VID},  {".avi", VID}, {".wmv", VID},  {".ts", VID},   {".json", JSON}};
const QRegularExpression IMG_PILE_NAME_PATTERN{"^(.*?)( | - )?(\\d{1,3})?$"};
const QRegularExpression VID_PILE_NAME_PATTERN{"^(.*?)( | - )?(HD|FHD|4k|720p|1080p|2160p|810p|60FPS|otherRes)?$",
                                               QRegularExpression::PatternOption::CaseInsensitiveOption};

bool ExtractPileItemsOutFolder::CanExtractOut(const QStringList& items) {
  QString stdName;

  QString noNumberName;
  QRegularExpressionMatch result;
  for (const QString& medName : items) {
    QString baseName, ext;
    std::tie(baseName, ext) = PATHTOOL::GetBaseNameExt(medName);
    noNumberName = baseName;
    auto typeEnum = DOT_EXT_2_TYPE.value(ext.toLower(), SCENE_COMPONENT_TYPE::OTHER);
    switch (typeEnum) {
      case IMG: {
        if ((result = IMG_PILE_NAME_PATTERN.match(baseName)).hasMatch()) {
          noNumberName = result.captured(1);
        }
        break;
      }
      case VID: {
        if ((result = VID_PILE_NAME_PATTERN.match(baseName)).hasMatch()) {
          noNumberName = result.captured(1);
        }
        break;
      }
      case JSON:
      case OTHER:
        break;
    }
    if (stdName.isEmpty()) {
      stdName = noNumberName;
      continue;
    }
    if (stdName != noNumberName) {
      return false;
    }
  }
  return true;
}

int ExtractPileItemsOutFolder::operator()(const QMap<QString, QStringList>& folder2PileItems) {
  int foldersNeedExtractCnt = 0, itemsExtractedOutCnt = 0;
  for (auto it = folder2PileItems.cbegin(); it != folder2PileItems.cend(); ++it) {
    const QStringList& items = it.value();
    if (items.isEmpty()) {
      // this folder may be redundant, will not process here
      continue;
    }
    if (!CanExtractOut(items)) {
      qDebug("path[%s] should not extract out", qPrintable(it.key()));
      continue;
    }
    qDebug("Extract %d pile item(s) out of folder[%s]", items.size(), qPrintable(it.key()));
    itemsExtractedOutCnt += items.size();
    ++foldersNeedExtractCnt;
  }
  qDebug("Extract %d item(s) total from %d folders", itemsExtractedOutCnt, foldersNeedExtractCnt);
  return foldersNeedExtractCnt;
}

int ExtractPileItemsOutFolder::operator()(const QString& rootPath) {
  QMap<QString, QStringList> folder2PileItems;
  QDir rootPathDir(rootPath, "", QDir::SortFlag::Name, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  for (const QString& path : rootPathDir.entryList()) {
    QDir pathDir{rootPath + '/' + path, "", QDir::SortFlag::NoSort, QDir::Filter::Files};
    if (!pathDir.isEmpty(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot)) {
      qDebug("skip, folder existed in path[%s]", qPrintable(pathDir.absolutePath()));
      continue;
    }
    folder2PileItems[path] = pathDir.entryList();
  }
  return operator()(folder2PileItems);
}

#include "PublicVariable.h"
int ScenesMixed::operator()(const QString& path) {
  QDir mediaDir(path, "", QDir::SortFlag::Name, QDir::Filter::Files);
  mediaDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET + TYPE_FILTER::IMAGE_TYPE_SET + TYPE_FILTER::JSON_TYPE_SET);
  return operator()(mediaDir.entryList());
}

int ScenesMixed::operator()(const QStringList& files) {
  QString noNumberName;
  QRegularExpressionMatch result;
  for (const QString& medName : files) {
    QString baseName, ext;
    std::tie(baseName, ext) = PATHTOOL::GetBaseNameExt(medName);
    noNumberName = baseName;
    auto typeEnum = DOT_EXT_2_TYPE.value(ext.toLower(), SCENE_COMPONENT_TYPE::OTHER);
    switch (typeEnum) {
      case IMG: {
        if ((result = IMG_PILE_NAME_PATTERN.match(baseName)).hasMatch()) {
          noNumberName = result.captured(1);
        }
        m_img2Name[noNumberName].append(medName);
        break;
      }
      case VID: {
        if ((result = VID_PILE_NAME_PATTERN.match(baseName)).hasMatch()) {
          noNumberName = result.captured(1);
        }
        m_vid2Name[noNumberName].append(medName);
        break;
      }
      case JSON:
        m_json2Name[baseName] = medName;
        break;
      case OTHER:
        break;
    }
  }

  for (auto& pr : m_img2Name) {
    std::sort(pr.begin(), pr.end(), [](const QString& lhs, const QString& rhs) -> bool { return lhs.size() < rhs.size(); });
  }

  qDebug("%d,%d,%d piles of img/vid/json found from %d item(s) given", m_img2Name.size(), m_vid2Name.size(), m_json2Name.size(), files.size());
  return m_json2Name.size();
}

const QString& ScenesMixed::GetFirstImg(const QString& baseName) const {
  auto it = m_img2Name.find(baseName);
  if (it == m_img2Name.cend()) {
    static QString imgNoExist;
    return imgNoExist;
  }
  return it.value().first();
}
const QString& ScenesMixed::GetFirstVid(const QString& baseName) const {
  auto it = m_vid2Name.find(baseName);
  if (it == m_vid2Name.cend()) {
    static QString vidNoExist;
    return vidNoExist;
  }
  return it.value().first();
}
