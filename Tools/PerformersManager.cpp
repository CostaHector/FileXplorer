#include "PerformersManager.h"
#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"

#include <QDir>
#include <QDirIterator>

PerformersManager::PerformersManager() : m_performers(ReadOutPerformers()), perfsCompleter(m_performers.values()) {
  perfsCompleter.setCaseSensitivity(Qt::CaseInsensitive);
  perfsCompleter.setCompletionMode(QCompleter::CompletionMode::PopupCompletion);
}

PerformersManager& PerformersManager::getIns() {
  static PerformersManager ins;
  qDebug("PerformersManager::getIns()");
  return ins;
}

QSet<QString> PerformersManager::ReadOutPerformers() {
#ifdef _WIN32
  const QString& perfFilePath = PreferenceSettings().value(MemoryKey::WIN32_PERFORMERS_TABLE.name).toString();
#else
  const QString& perfFilePath = PreferenceSettings().value(MemoryKey::LINUX_PERFORMERS_TABLE.name).toString();
#endif

  QFile performersFi(perfFilePath);
  if (not performersFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("file[%s] not found.", qPrintable(performersFi.fileName()));
    return {};
  }
  QTextStream stream(&performersFi);
  stream.setCodec("UTF-8");
  decltype(m_performers) perfSet;
  while (not stream.atEnd()) {
    perfSet.insert(stream.readLine().toLower());
  }
  if (perfSet.contains("")) {
    perfSet.remove("");
  }
  performersFi.close();
  qDebug("%d performers read out", perfSet.size());
  return perfSet;
}

int PerformersManager::ForceReloadPerformers() {
  int beforeStudioNameCnt = m_performers.size();
  m_performers = PerformersManager::ReadOutPerformers();
  int afterStudioNameCnt = m_performers.size();
  qDebug("%d performers added/removed", afterStudioNameCnt - beforeStudioNameCnt);
  return afterStudioNameCnt - beforeStudioNameCnt;
}

int PerformersManager::LearningFromAPath(const QString& path) {
  if (not QDir(path).exists()) {
    return 0;
  }

  const int beforePerformersCnt = m_performers.size();
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = JsonFileHelper::MovieJsonLoader(jsonPath);
    if (not dict.contains(DB_HEADER_KEY::Performers)) {
      continue;
    }
    const QVariant& v = dict[DB_HEADER_KEY::Performers];
    for (const QString& performer : v.toStringList()) {
      if (performer.isEmpty() or m_performers.contains(performer)) {
        continue;
      }
      m_performers.insert(performer.toLower());
    }
  }

  const int increCnt = int(m_performers.size()) - beforePerformersCnt;
  qDebug("Learn extra %d performers, now %u performers in total", increCnt, m_performers.size());

  QFile performersFi(PROJECT_PATH + "/bin/PERFORMERS_TABLE.txt");
  if (not performersFi.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug("file cannot open. learned performers will not update to %s.", qPrintable(performersFi.fileName()));
  }
  QStringList perfsLst(m_performers.cbegin(), m_performers.cend());
  QTextStream stream(&performersFi);
  stream.setCodec("UTF-8");
  stream << perfsLst.join("\n");
  performersFi.close();
  return increCnt;
}

QStringList PerformersManager::SplitSentence(QString sentence) {
  if (sentence.isEmpty()) {
    return {};
  }
  using namespace JSON_RENAME_REGEX;
  sentence.replace(DISCRAD_LETTER_COMP, " ");
  sentence.replace(AND_COMP, " & ");
  sentence.remove(RESOLUTION_COMP);
  return sentence.split(CONTINOUS_SPACE);
}

auto PerformersManager::RmvBelongLetter(const QString& word) -> QString {
  QString s = word.trimmed();
  if (s.endsWith("'s")) {
    s.chop(2);
  } else if (s.endsWith("'")) {
    s.chop(1);
  }
  return s;
};

QStringList PerformersManager::FilterPerformersOut(const QStringList& words) const {
  if (words.isEmpty()) {
    return {};
  }
  QStringList performersList;
  int i = 0;
  const int N = words.size();
  while (i < N) {
    if (i < N - 2) {
      const QString& w3 = words[i] + " " + words[i + 1] + " " + RmvBelongLetter(words[i + 2]);
      if (m_performers.contains(w3.toLower())) {
        if (not performersList.contains(w3))
          performersList.append(w3);
        i += 3;
        continue;
      }
    }
    if (i < N - 1) {
      const QString& w2 = words[i] + " " + RmvBelongLetter(words[i + 1]);
      if (m_performers.contains(w2.toLower())) {
        if (not performersList.contains(w2)) {
          performersList.append(w2);
        }
        i += 2;
        continue;
      }
    }
    const QString& w1 = RmvBelongLetter(words[i]);
    if (not w1.isEmpty() and m_performers.contains(w1.toLower())) {
      if (not performersList.contains(w1))
        performersList.append(w1);
      i += 1;
      continue;
    }
    ++i;
  }
  performersList.removeDuplicates();
  return performersList;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__

int main(int argc, char* argv[]) {
  auto& performersIns = PerformersStringParser::getIns();
  //  performersIns.LearningFromAPath("E:/115/test");
  const auto& wordsList = performersIns.SplitSentence("Next Door Originals - Rivals Waiter vs Waiter - Theo Brady & Devin Franco flip-fuck BB 1080p");
  const auto& perfsList = performersIns.FilterPerformersOut(wordsList);
  qDebug() << perfsList;
  return 0;
}
#endif
