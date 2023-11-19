#include "ProductionStudioManager.h"

#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"

#include <QDir>
#include <QDirIterator>
#include <QTextStream>

ProductionStudioManager::ProductionStudioManager() : m_prodStudioMap(JsonFileHelper::MovieJsonLoader(":/STANDARD_STUDIO_NAME_JSON")) {}

int ProductionStudioManager::LearningFromAPath(const QString& path) {
  if (not QDir(path).exists()) {
    return 0;
  }
  const int beforePerformersCnt = m_prodStudioMap.size();
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = JsonFileHelper::MovieJsonLoader(jsonPath);
    if (not dict.contains(JSONKey::ProductionStudio)) {
      continue;
    }
    const QString& v = dict[JSONKey::ProductionStudio].toString();
    if (v.isEmpty() or m_prodStudioMap.contains(v.toLower())){
      continue;
    }
    m_prodStudioMap.insert(v.toLower(), v);
  }

  const int increCnt = int(m_prodStudioMap.size()) - beforePerformersCnt;
  qDebug("Learn extra %d production studios, now %u production studios in total", increCnt, m_prodStudioMap.size());

  const bool dumpRes = JsonFileHelper::MovieJsonDumper(m_prodStudioMap, PROJECT_PATH + "/bin/STANDARD_STUDIO_NAME_JSON.txt");
  return increCnt;
}

ProductionStudioManager& ProductionStudioManager::getIns() {
  static ProductionStudioManager ins;
  qDebug("ProductionStudioManager::getIns()");
  return ins;
}

auto ProductionStudioManager::operator()(QString sentence) const -> QString {
  using namespace JSON_RENAME_REGEX;
  sentence.remove(leadingStrComp); // remove [FFL], [FL], [GT]
  sentence.remove(leadingOpenBracketComp); // remove open braces [({
  sentence.replace(closedBracketComp, "-");

  QString prodStudioSection = sentence.split("-")[0];
  QString noInvalidStr = prodStudioSection.remove(keepComp);
  const QString& inputStr = noInvalidStr.trimmed();
  return this->operator[](inputStr);
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  auto& performersIns = ProductionStudioManager::getIns();
  //  qDebug() << performersIns.m_prodStudioMap;
//  qDebug() << performersIns("[FFL] Lucas Entertainment - ABC.mp4");
  qDebug() << performersIns("[BaitBus] 2008 - Part.mp4");
  return 0;
}
#endif
