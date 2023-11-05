#include "PerformersManager.h"
#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"

#include <QDir>
#include <QDirIterator>
PerformersManager::PerformersManager() : performers(loadExistedPerformers()) {}

QSet<QString> PerformersManager::loadExistedPerformers() {
  QFile performersFi(":/PERFORMERS_TABLE");
  if (not performersFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("file[%s] not found. loadExistedPerformers abort", performersFi.fileName().toStdString().c_str());
    return {};
  }
  QTextStream stream(&performersFi);
  stream.setCodec("UTF-8");
  QSet<QString> st;
  while (not stream.atEnd()) {
    st.insert(stream.readLine().toLower());
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
        performers.insert(performer.toString().toLower());
      }
    }
  }

  const int increCnt = int(performers.size()) - beforePerformersCnt;
  qDebug("Learn extra %d performers, now %u performers in total", increCnt, performers.size());

  QFile performersFi(PROJECT_PATH + "/bin/PERFORMERS_TABLE.txt");
  if (not performersFi.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug("file cannot open. learned performers will not update to %s.", performersFi.fileName().toStdString().c_str());
  }
  QStringList perfsLst(performers.cbegin(), performers.cend());
  QTextStream stream(&performersFi);
  stream.setCodec("UTF-8");
  stream << perfsLst.join("\n");
  return increCnt;
}

PerformersManager& PerformersManager::getIns() {
  static PerformersManager ins;
  qDebug("PerformersManager::getIns()");
  return ins;
}

QStringList PerformersManager::MovieNameWordsSplit(QString sentence) const {
  if (sentence.isEmpty()) {
    return {};
  }
  static QRegExp keepComp("[^A-Z0-9._@# ']", Qt::CaseInsensitive);
  static QRegExp andComp(" and | fucked by | fucked | fucks | fuck ", Qt::CaseInsensitive);
  static QRegExp resolutionComp("1080p|720p|480p|810p|4K|FHD|HD|SD", Qt::CaseInsensitive);
  static QRegExp continousSpace("\\s+");
  QString valuableString = sentence.replace(keepComp, "");
  QString noActionString = valuableString.replace(andComp, "&");
  QString noResolutionString = noActionString.replace(resolutionComp, "");
  return noResolutionString.split(continousSpace);
}

QStringList PerformersManager::PeformersFilterOut(const QStringList& words) const {
  if (words.isEmpty()) {
    return {};
  }
  QStringList performersList;
  int i = 0;
  const int N = words.size();
  while (i < N) {
    if (i < N - 2) {
      const QString& w3 = words[i] + " " + words[i + 1] + " " + words[i + 2];
      if (performers.contains(w3.toLower())) {
        if (not performersList.contains(w3))
          performersList.append(w3);
        i += 3;
        continue;
      }
    }
    if (i < N - 1) {
      const QString& w2 = words[i] + " " + words[i + 1];
      if (performers.contains(w2.toLower())) {
        if (not performersList.contains(w2))
          performersList.append(w2);
        i += 2;
        continue;
      }
    }
    const QString& w1 = words[i];
    if (performers.contains(w1.toLower())) {
      if (not performersList.contains(w1))
        performersList.append(w1);
      i += 1;
      continue;
    }
    ++i;
  }
  return performersList;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  auto& performersIns = PerformersManager::getIns();
  //  performersIns.LearningFromAPath("E:/115/test");
  const auto& wordsList =
      performersIns.MovieNameWordsSplit("Next Door Originals - Rivals Waiter vs Waiter - Theo Brady & Devin Franco flip-fuck BB 1080p");
  const auto& perfsList = performersIns.PeformersFilterOut(wordsList);
  qDebug() << perfsList;
  return 0;
}
#endif
