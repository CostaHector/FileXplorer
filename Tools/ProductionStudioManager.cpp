#include "ProductionStudioManager.h"

#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"

#include <QDir>
#include <QDirIterator>
#include <QTextStream>

ProductionStudioManager::ProductionStudioManager() : m_prodStudioMap(ReadOutStdStudioName()) {}

QVariantHash ProductionStudioManager::ReadOutStdStudioName() {
#ifdef _WIN32
  const QString stdStudiosFilePath = PreferenceSettings().value(MemoryKey::WIN32_STANDARD_STUDIO_NAME.name).toString();
#else
  const QString stdStudiosFilePath = PreferenceSettings().value(MemoryKey::LINUX_STANDARD_STUDIO_NAME.name).toString();
#endif
  QFile txtFile(stdStudiosFilePath);
  if (!txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("File[%s] not found or open for read failed", qPrintable(stdStudiosFilePath));
    return {};
  }
  QVariantHash stdStudioNameDict;

  QTextStream in(&txtFile);
  in.setCodec("UTF-8");
  int lineIndex = 0;
  while (!in.atEnd()) {
    ++lineIndex;
    QString line = in.readLine(128);
    if (line.isEmpty()) {
      continue;
    }
    int tabKeyInd = line.indexOf('\t');
    if (tabKeyInd == -1) {
      qWarning("The %dth line of file[%s] is invalid", lineIndex, qPrintable(stdStudiosFilePath));
      continue;
    }
    stdStudioNameDict.insert(line.left(tabKeyInd), line.mid(tabKeyInd + 1));
  }
  txtFile.close();
  qDebug("%d studio item(s) read out from %d lines", stdStudioNameDict.size(), lineIndex);
  return stdStudioNameDict;
}

int ProductionStudioManager::ForceReloadStdStudioName() {
  int befCnt = m_prodStudioMap.size();
  auto newStudioNames = ProductionStudioManager::ReadOutStdStudioName();
  m_prodStudioMap.swap(newStudioNames);
  int aftCnt = m_prodStudioMap.size();
  qDebug("standard studio names rule from %d to %d", befCnt, aftCnt);
  return aftCnt - befCnt;
}

int ProductionStudioManager::LearningFromAPath(const QString& path) {
  if (not QDir(path).exists()) {
    return 0;
  }

  QHash<QString, QString> studiosIncrementMap;
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = JsonFileHelper::MovieJsonLoader(jsonPath);
    if (!dict.contains(JSONKey::ProductionStudio)) {
      continue;
    }
    const QString& v = dict[JSONKey::ProductionStudio].toString();
    for (const QString& psFrom : StandardProductionStudioFrom(v)) {
      if (psFrom.isEmpty() || m_prodStudioMap.contains(psFrom)) {
        continue;
      }
      studiosIncrementMap.insert(psFrom, v);
    }
  }
  qDebug("Learn extra %d studios from json files", studiosIncrementMap.size());
  if (studiosIncrementMap.isEmpty()) {
    return studiosIncrementMap.size();
  }
#ifdef _WIN32
  const QString stdStudiosFilePath = PreferenceSettings().value(MemoryKey::WIN32_STANDARD_STUDIO_NAME.name).toString();
#else
  const QString stdStudiosFilePath = PreferenceSettings().value(MemoryKey::LINUX_STANDARD_STUDIO_NAME.name).toString();
#endif
  QFile txtFile(stdStudiosFilePath);
  if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
    qDebug("File[%s] not found or open for write failed", qPrintable(stdStudiosFilePath));
    return {};
  }
  QTextStream in(&txtFile);
  in.setCodec("UTF-8");
  for (auto it = studiosIncrementMap.cbegin(), end = studiosIncrementMap.cend(); it != end; ++it) {
    in << it.key() << '\t' << it.value() << '\n';
  }
  in.flush();
  txtFile.close();
  return studiosIncrementMap.size();
}

ProductionStudioManager& ProductionStudioManager::getIns() {
  static ProductionStudioManager ins;
  qDebug("ProductionStudioManager::getIns()");
  return ins;
}

using namespace JSON_RENAME_REGEX;
QStringList ProductionStudioManager::StandardProductionStudioFrom(QString standardPs) const {
  // Both "lucas entertainment" and "lucasentertainment" should get "LucasEntertainment"
  // so, "LucasEntertainment" is from "lucas entertainment" or "lucasentertainment"
  const QString& pslower = standardPs.toLower();
  const QString& psWithSpace = standardPs.replace(SPLIT_BY_UPPERCASE, " \\1").trimmed();
  return {pslower, psWithSpace.toLower()};
}

QString ProductionStudioManager::FileName2StudioNameSection(QString sentence) const {
  sentence.remove(leadingStrComp);          // remove [FFL], [FL], [GT]
  sentence.remove(leadingOpenBracketComp);  // remove open braces [({
  sentence.replace(nonLeadingBracketComp, "-");

  QString prodStudioSection = sentence.split("-")[0];
  QString noInvalidStr = prodStudioSection.remove(DISCRAD_LETTER_COMP);
  const QString& studioNameSection = noInvalidStr.trimmed();
  return studioNameSection;
}

auto ProductionStudioManager::operator()(const QString& sentence) const -> QString {
  const QString studioNameSection = FileName2StudioNameSection(sentence);
  return this->operator[](studioNameSection);
}

QString ProductionStudioManager::hintStdStudioName(const QString& sentence) const {
  const QString& studioNameSection = FileName2StudioNameSection(sentence);
  const QString& lowercaseStudioName = studioNameSection.toLower();
  auto it = m_prodStudioMap.find(lowercaseStudioName);
  return it != m_prodStudioMap.cend() ? it.value().toString() : "";
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  auto& performersIns = ProductionStudioManager::getIns();
  //  qDebug() << performersIns.m_prodStudioMap;
  //  qDebug() << performersIns("[FFL] Lucas Entertainment - ABC.mp4");
  qDebug() << performersIns("[BaitBus] 2008 - Part.mp4");
  return 0;
}
#endif
