#include "PerformersAkaManager.h"
#include "PerformerJsonFileHelper.h"
#include "TableFields.h"
#include "MemoryKey.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "StringTool.h"
#include <QSqlField>
#include <QTextStream>
#include <QFile>

PerformersAkaManager& PerformersAkaManager::getIns() {
  static PerformersAkaManager ins;
  return ins;
}

PerformersAkaManager::PerformersAkaManager() : m_akaPerf(ReadOutAkaName()) {}

QHash<QString, QString> PerformersAkaManager::ReadOutAkaName() {
  using namespace PathTool::FILE_REL_PATH;
  static const QString akaPerfFilePath = PathTool::GetPathByApplicationDirPath(AKA_PERFORMERS);
  QFile file{akaPerfFilePath};
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("File not found: %s.", qPrintable(file.fileName()));
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
  qDebug("%d aka name(s) read out", akaDict.size());
  return akaDict;
}

int PerformersAkaManager::ForceReloadAkaName() {
  int beforeAkaNameCnt = m_akaPerf.size();
  m_akaPerf = PerformersAkaManager::ReadOutAkaName();
  int afterAkaNameCnt = m_akaPerf.size();
  qDebug("%d aka names added/removed", afterAkaNameCnt - beforeAkaNameCnt);
  return afterAkaNameCnt - beforeAkaNameCnt;
}

