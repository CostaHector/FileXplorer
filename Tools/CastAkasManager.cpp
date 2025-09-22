#include "CastAkasManager.h"
#include "CastPsonFileHelper.h"
#include "TableFields.h"
#include "MemoryKey.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StringTool.h"
#include <QTextStream>
#include <QRegularExpression>
#include <QFile>

template class SingletonManager<CastAkasManager, CAST_AKA_MGR_DATA_T>;


CastAkasManager::CastAkasManager() {
#ifndef RUNNING_UNIT_TESTS
  using namespace PathTool::FILE_REL_PATH;
  const QString defaultPath = PathTool::GetPathByApplicationDirPath(AKA_PERFORMERS);
  InitializeImpl(defaultPath);

#endif
}
void CastAkasManager::InitializeImpl(const QString& path) {
  mLocalFilePath = path;
  CastAkaMap() = ReadOutCastAkas();
}

QHash<QString, QString> CastAkasManager::ReadOutCastAkas() const {
  QFile file{mLocalFilePath};
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_D("File not found: %s.", qPrintable(file.fileName()));
    return {};
  }

  QHash<QString, QString> akaDict;
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  static const QRegularExpression PERF_SPLIT("\\s*,\\s*");
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    line.replace(PERF_SPLIT, "|");
    for (const QString& perf : line.split('|')) {
      akaDict.insert(perf, line);
    }
  }
  file.close();
  LOG_D("%d aka name(s) read out", akaDict.size());
  return akaDict;
}

int CastAkasManager::ForceReloadImpl() {
  int beforeAkaNameCnt = CastAkaMap().size();
  CAST_AKA_MGR_DATA_T tmp = CastAkasManager::ReadOutCastAkas();
  CastAkaMap().swap(tmp);
  int afterAkaNameCnt = CastAkaMap().size();
  LOG_D("%d aka names added/removed", afterAkaNameCnt - beforeAkaNameCnt);
  return afterAkaNameCnt - beforeAkaNameCnt;
}

#ifdef RUNNING_UNIT_TESTS
int CastAkasManager::ResetStateForTestImpl(const QString& localFilePath) {
  InitializeImpl(localFilePath);
  return 0;
}
#endif
