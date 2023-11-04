#include "PerformersManager.h"
#include "Tools/JsonFileHelper.h"

#include <QDir>
#include <QDirIterator>
PerformersManager::PerformersManager() : performers(loadExistedPerformers()) {}

QSet<QString> PerformersManager::loadExistedPerformers() {
  QFile performersFi(":/PERFORMERS_TABLE");
  if (not performersFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("file not found");
    return {};
  }
  QTextStream stream(&performersFi);
  stream.setCodec("UTF-8");
  QSet<QString> st;
  while (not stream.atEnd()) {
    st.insert(stream.readLine());
  }
  performersFi.close();
  qDebug("Load %d performers succeed", st.size());
  return st;
}

int PerformersManager::LearningFromAPath(const QString& path) {
  if (not QDir(path).exists()) {
    return 0;
  }

  const int beforePerformersCnt = performers.size();
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QHash<QString, QJsonValue>& dict = JsonFileHelper::MovieJsonLoader(jsonPath);
    if (not dict.contains("Performers")) {
      continue;
    }
    const QJsonValue& v = dict["Performers"];
    if (not v.isArray()) {
      continue;
    }
    for (const QJsonValue& performer : v.toArray()) {
      if (performer.isString() and not performers.contains(performer.toString())) {
        performers.insert(performer.toString());
      }
    }
  }

  const int increCnt = int(performers.size()) - beforePerformersCnt;
  qDebug("Learn extra %d performers, now %u performers in total", increCnt, performers.size());

  QFile performersFi(":/PERFORMERS_TABLE");
  if (performersFi.isWritable() and performersFi.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QStringList perfsLst(performers.cbegin(), performers.cend());
    QTextStream stream(&performersFi);
    stream.setCodec("UTF-8");
    stream << perfsLst.join("\r\n");
  } else {
    qDebug("file not writable or not found. learned performers will not update to %s.", performersFi.fileName().toStdString().c_str());
  }

  return increCnt;
}

PerformersManager& PerformersManager::getIns() {
  static PerformersManager ins;
  return ins;
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  auto& performersIns = PerformersManager::getIns();
  performersIns.LearningFromAPath("E:/115/test");
  return 0;
}
#endif
