#include "Categorizer.h"

#include "PublicVariable.h"
#include "Tools/PathTool.h"
#include "Tools/ItemsPileCategory.h"
#include "UndoRedo.h"

#include <QDir>
#include <QMap>
#include <QRegularExpression>

QMap<QString, QStringList> Categorizer::ClassifyItemIntoPiles(const QStringList& fileFolderMixed) {
  using namespace ItemsPileCategory;
  QMap<QString, QStringList> baseName2ItemsPile;
  QString noNumberName;
  QRegularExpressionMatch result;
  for (const QString& medName : fileFolderMixed) {
    QString baseName, ext;
    std::tie(baseName, ext) = PATHTOOL::GetBaseNameExt(medName);
    noNumberName = baseName;
    auto typeEnum = DOT_EXT_2_TYPE.value(ext.toLower(), SCENE_COMPONENT_TYPE::OTHER);
    switch (typeEnum) {
      case IMG: {
        if ((result = IMG_PILE_NAME_PATTERN.match(baseName)).hasMatch()) {
          noNumberName = result.captured(1);
        }
        baseName2ItemsPile[noNumberName].append(medName);
        break;
      }
      case VID: {
        if ((result = VID_PILE_NAME_PATTERN.match(baseName)).hasMatch()) {
          noNumberName = result.captured(1);
        }
        baseName2ItemsPile[noNumberName].append(medName);
        break;
      }
      case JSON:
        baseName2ItemsPile[baseName].append(medName);
        break;
      case OTHER:  // can be a folder
        baseName2ItemsPile[baseName].append(medName);
        break;
    }
  }
  return baseName2ItemsPile;
}

QMap<QString, QStringList> Categorizer::ClassifyItemIntoPiles(const QString& path) {
  QDir pathdir(path, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  return ClassifyItemIntoPiles(pathdir.entryList());
}

int Categorizer::operator()(const QString& path, const QMap<QString, QStringList>& pilesMap) {
  int filesRearrangedCnt = 0;
  int pathCreatedCnt = 0;
  for (auto it = pilesMap.cbegin(); it != pilesMap.cend(); ++it) {
    const QString& folderName = it.key();
    const QStringList& files = it.value();
    if (files.size() < 2) {
      qDebug("FolderName[%s] only contains %d item(s), skip", qPrintable(folderName), files.size());
      continue;
    }
    const QString& underPath = path + '/' + folderName;
    QDir underDir(underPath);
    if (!underDir.exists()) {
      // create a folder path + '/' + folderName
      m_cmds.append({"mkpath", path, folderName});
      ++pathCreatedCnt;
    }
    for (const QString& file : files) {
      if (QFileInfo(path, file).isDir()) {
        // path/file is a directory, skip move "path/file" to "underPath/*"
        continue;
      }
      if (underDir.exists(file)) {
        qDebug("%s/%s already exist, move will failed, skip it", qPrintable(underPath), qPrintable(file));
        continue;
      }
      m_cmds.append({"rename", path, file, underPath, file});
      ++filesRearrangedCnt;
    }
  }

  qDebug("%d file(s) rearrange, %d path(s) make, %d cmds been generated under path[%s]", filesRearrangedCnt, pathCreatedCnt, qPrintable(path));
  return filesRearrangedCnt;
}

int Categorizer::operator()(const QString& path) {
  if (!QFileInfo(path).isDir()) {
    qDebug("path[%s] is not an existed directory", qPrintable(path));
    return {};
  }
  const auto& pilesMap = ClassifyItemIntoPiles(path);
  return operator()(path, pilesMap);
}

bool Categorizer::StartToRearrange() {
  const auto isAllSuccess = g_undoRedo.Do(m_cmds);
  qDebug("%d rearrange cmd(s) execute result: bool[%d]", m_cmds.size(), isAllSuccess);
  return isAllSuccess;
}
