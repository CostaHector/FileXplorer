#include "SceneInfoManager.h"
#include "PublicVariable.h"
#include "SceneMixed.h"
#include "JsonHelper.h"
#include "PathTool.h"
#include "Logger.h"
#include "JsonUpdater.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QDateTime>

namespace SceneInfoManager {

QString ScnMgr::GetScnAbsFilePath(const QString& folderPath) {
  return folderPath + '/' + PathTool::fileName(folderPath) + ".scn";
}

JsonOp::Counter ScnMgr::UpdateJsonUnderAPath(const QString& path) {
  JsonOp::Counter counter;
  if (!QFileInfo(path).isDir()) {
    return counter;
  }
  int jsonUpdatedCnt = 0, jsonUsedCnt = 0;
  int imgNameKeyFieldUpdatedCnt = 0, vidNameKeyFieldUpdatedCnt = 0;

  ScenesMixed sMixed;
  sMixed(path);

  for (const QString& jsonFileBaseName: sMixed.m_json2Name) {
    const QString jPath{path + '/' + jsonFileBaseName + ".json"};
    QVariantHash rawJsonDict = JsonHelper::MovieJsonLoader(jPath);
    JsonOp::Counter currentCounter = JsonUpdater::UpdateJsonKeyValuePair(sMixed, rawJsonDict, path, jsonFileBaseName);
    if (currentCounter.m_jsonUsedCnt == 0) { // useless json found
      continue;
    }
    counter += currentCounter;
    if (currentCounter.m_jsonUpdatedCnt != 0) {
      JsonHelper::DumpJsonDict(rawJsonDict, jPath);
    }
  }
  return counter;
}

JsonOp::Counter ScnMgr::operator()(const QString& rootPath) {  // will iterate all sub
  if (!QFileInfo(rootPath).isDir()) {
    LOG_D("Not an existed directory[%s]", qPrintable(rootPath));
    return JsonOp::Counter{};
  }
  JsonOp::Counter cnt{0, 0, 0, 0};
  QDirIterator folderIt{rootPath, {}, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDirIterator::IteratorFlag::Subdirectories};
  while (folderIt.hasNext()) {
    cnt += UpdateJsonUnderAPath(folderIt.next());
  }
  cnt += UpdateJsonUnderAPath(rootPath);
  return cnt;
}

int ScnMgr::UpdateScnFiles(const QString& rootPath) {
  QDirIterator jsonIt{rootPath, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  QMap<QString, SceneInfoList> folder2Scenes;  // relativePathToJsonFile -> scenes
  int scnTotalCnt = 0;
  while (jsonIt.hasNext()) {
    QString jsonAbsPath = jsonIt.next();
    QVariantHash jsonHash = JsonHelper::MovieJsonLoader(jsonAbsPath);
    if (jsonHash.isEmpty()) {
      continue;
    }
    folder2Scenes[PathTool::absolutePath(jsonAbsPath)].push_back(SceneInfo::fromJsonVariantHash(jsonHash));
    ++scnTotalCnt;
  }

  if (scnTotalCnt == 0) {
    LOG_D("json dicts not exist at all under[%s], skip write", qPrintable(rootPath));
    return 0;
  }

  int scnFilesGeneratedCnt = 0;
  for (auto it = folder2Scenes.cbegin(); it != folder2Scenes.cend(); ++it) {
    const QString& scnAbsFolderPath = it.key();
    const SceneInfoList& scenes = it.value();

    LOG_D("%d scenes(s) under[%s] are found to generate a scn file", scenes.size(), qPrintable(scnAbsFolderPath));
    const QString& scnAbsFilePath{GetScnAbsFilePath(scnAbsFolderPath)};
    if (!SceneHelper::SaveScenesListToBinaryFile(scnAbsFilePath, scenes)) {
      continue;
    }
    ++scnFilesGeneratedCnt;
  }
  LOG_D("%d scn file(s) generated above", scnFilesGeneratedCnt);
  return scnFilesGeneratedCnt;
}

int ScnMgr::ClearScnFiles(const QString& rootPath) {
  QDirIterator folderIt{rootPath, {"*.scn"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  int scnTotalCnt = 0, deleteOkCnt = 0;
  while (folderIt.hasNext()) {
    deleteOkCnt += QFile::remove(folderIt.next());
    ++scnTotalCnt;
  }
  LOG_D("%d/%d scn file under[%s] delete succeed", deleteOkCnt, scnTotalCnt, qPrintable(rootPath));
  return deleteOkCnt;
}

}  // namespace SceneInfoManager
