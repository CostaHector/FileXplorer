#include "ExtractPileItemsOutFolder.h"
#include "Tools/PathTool.h"
#include "Tools/ItemsPileCategory.h"
#include "UndoRedo.h"
#include <QRegularExpression>
#include <QDir>

using namespace ItemsPileCategory;
using namespace FileOperatorType;

QMap<QString, QStringList> ExtractPileItemsOutFolder::UnpackItemFromPiles(const QString& path) {
  QDir rootPathDir(path, "", QDir::SortFlag::Name, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  return UnpackItemFromPiles(path, rootPathDir.entryList());
}
QMap<QString, QStringList> ExtractPileItemsOutFolder::UnpackItemFromPiles(const QString& path, const QStringList& folders) {
  QMap<QString, QStringList> folder2PileItems;
  for (const QString& subPath : folders) {
    QDir pathDir{path + '/' + subPath, "", QDir::SortFlag::NoSort, QDir::Filter::Files};
    if (!pathDir.isEmpty(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot)) {
      qDebug("skip, folder existed in path[%s]", qPrintable(pathDir.absolutePath()));
      continue;
    }
    folder2PileItems[subPath] = pathDir.entryList();
  }
  return folder2PileItems;
}

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

int ExtractPileItemsOutFolder::operator()(const QString& path, const QMap<QString, QStringList>& folder2PileItems) {
  QDir outDir(path);
  int foldersNeedExtractCnt = 0, itemsExtractedOutCnt = 0;
  for (auto it = folder2PileItems.cbegin(); it != folder2PileItems.cend(); ++it) {
    const QString& folderName = it.key();
    const QStringList& files = it.value();
    if (files.size() < 2) {
      // this folder is not classfied by Categorizer will not process here
      qDebug("FolderName[%s] only contains %d item(s), skip", qPrintable(folderName), files.size());
      continue;
    }
    if (!CanExtractOut(files)) {
      qDebug("path[%s] should not extract out", qPrintable(it.key()));
      continue;
    }
    int filesExtractedCnt = 0;
    for (const QString& file : files) {
      if (outDir.exists(file)) {
        qDebug("%s/%s already exist outside, move will failed, skip it", qPrintable(path), qPrintable(file));
        continue;
      }
      m_cmds.append(ACMD{RENAME, {path + '/' + folderName, file, path, file}});
      ++filesExtractedCnt;
    }
    itemsExtractedOutCnt += filesExtractedCnt;
    // recycle path + '/' + folderName
    m_cmds.append(ACMD{MOVETOTRASH, {path, folderName}});
    ++foldersNeedExtractCnt;
    qDebug("Extract %d pile item(s) out of folder[%s]", files.size(), qPrintable(it.key()));
  }
  qDebug("Extract %d item(s) total from %d folders", itemsExtractedOutCnt, foldersNeedExtractCnt);
  return foldersNeedExtractCnt;
}

int ExtractPileItemsOutFolder::operator()(const QString& path) {
  const QMap<QString, QStringList>& folder2PileItems = UnpackItemFromPiles(path);
  return operator()(path, folder2PileItems);
}

bool ExtractPileItemsOutFolder::StartToRearrange() {
  const auto isAllSuccess = g_undoRedo.Do(m_cmds);
  qDebug("%d rearrange cmd(s) execute result: bool[%d]", m_cmds.size(), isAllSuccess);
  return isAllSuccess;
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
