#include "ExtractPileItemsOutFolder.h"
#include "public/PathTool.h"
#include "Tools/Classify/ItemsPileCategory.h"
#include "public/UndoRedo.h"
#include <QRegularExpression>
#include <QDir>

using namespace ItemsPileCategory;
using namespace FileOperatorType;

QMap<QString, QStringList> ExtractPileItemsOutFolder::GetFolder2ItemsMapByCurPath(const QString& path) {
  QDir rootPathDir(path, "", QDir::SortFlag::Name, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  QMap<QString, QStringList> folder2PileItems;
  for (const QString& subPath : rootPathDir.entryList()) {
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
    std::tie(baseName, ext) = PathTool::GetBaseNameExt(medName);
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
        noNumberName = baseName;
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

int ExtractPileItemsOutFolder::operator()(const QString& path,                                 //
                                          const QMap<QString, QStringList>& folder2PileItems,  //
                                          const QStringList& alreadyExistItems) {
  int foldersNeedExtractCnt = 0, itemsExtractedOutCnt = 0;
  for (auto it = folder2PileItems.cbegin(); it != folder2PileItems.cend(); ++it) {
    const QString& folderName = it.key();
    const QStringList& files = it.value();
    if (files.size() < 2) {
      // this folder is not classfied by ItemsClassifier will not process here
      qDebug("FolderName[%s] only contains %d item(s), skip", qPrintable(folderName), files.size());
      continue;
    }
    if (!CanExtractOut(files)) {
      qDebug("path[%s] should not extract out", qPrintable(it.key()));
      continue;
    }
    int filesExtractedCnt = 0;
    for (const QString& file : files) {
      if (alreadyExistItems.contains(file)) {
        qDebug("%s/%s already exist outside, move will failed, skip it", qPrintable(path), qPrintable(file));
        continue;
      }
      m_cmds.append(ACMD::GetInstMV(path + '/' + folderName, file, path));
      ++filesExtractedCnt;
    }
    itemsExtractedOutCnt += filesExtractedCnt;
    // recycle path + '/' + folderName
    m_cmds.append(ACMD::GetInstRMDIR(path, folderName));
    ++foldersNeedExtractCnt;
    qDebug("Extract %d pile item(s) out of folder[%s]", files.size(), qPrintable(it.key()));
  }
  qDebug("Extract %d item(s) total from %d folders", itemsExtractedOutCnt, foldersNeedExtractCnt);
  return foldersNeedExtractCnt;
}

int ExtractPileItemsOutFolder::operator()(const QString& path) {
  QDir dirIt{path, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot};
  const QStringList& alreadyExistItems = dirIt.entryList();
  const QMap<QString, QStringList>& folder2PileItems = GetFolder2ItemsMapByCurPath(path);
  return operator()(path, folder2PileItems, alreadyExistItems);
}

bool ExtractPileItemsOutFolder::StartToRearrange() {
  const auto isAllSuccess = g_undoRedo.Do(m_cmds);
  qDebug("%d rearrange cmd(s) execute result: bool[%d]", m_cmds.size(), isAllSuccess);
  return isAllSuccess;
}
