#include "ItemsClassifier.h"
#include "Tools/Classify/SceneMixed.h"
#include "public/UndoRedo.h"

#include <QDir>
#include <QMap>
#include <QRegularExpression>

int ItemsClassifier::operator()(const QString& path, const QMap<QString, QStringList>& pilesMap) {
  using namespace FileOperatorType;
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
      m_cmds.append(ACMD::GetInstMKPATH(path, folderName));
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
      m_cmds.append(ACMD::GetInstMV(path, file, underPath));
      ++filesRearrangedCnt;
    }
  }

  qDebug("%d file(s) rearrange, %d path(s) make under path[%s]", filesRearrangedCnt, pathCreatedCnt, qPrintable(path));
  return filesRearrangedCnt;
}

int ItemsClassifier::operator()(const QString& path) {
  QDir pathdir(path, "", QDir::SortFlag::Name, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  if (!pathdir.exists()) {
    qDebug("path[%s] is not an existed directory", qPrintable(path));
    return {};
  }
  ScenesMixed sMixed;
  const auto& pilesMap = sMixed(pathdir.entryList());
  return operator()(path, pilesMap);
}

bool ItemsClassifier::StartToRearrange() {
  const auto isAllSuccess = g_undoRedo.Do(m_cmds);
  qDebug("%d rearrange cmd(s) execute result: bool[%d]", m_cmds.size(), isAllSuccess);
  return isAllSuccess;
}
