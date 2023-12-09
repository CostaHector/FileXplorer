#include "PerformersManager.h"
#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"

#include <QDir>
#include <QDirIterator>

PerformersManager::PerformersManager() : m_performers(loadExistedPerformers()), perfsCompleter(m_performers.values()) {
  perfsCompleter.setCaseSensitivity(Qt::CaseInsensitive);
  perfsCompleter.setCompletionMode(QCompleter::CompletionMode::PopupCompletion);
}

QSet<QString> PerformersManager::loadExistedPerformers() {
  QFile performersFi(SystemPath::PERFORMERS_TABLE_TXT);
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
  if (st.contains("")) {
    st.remove("");
  }
  performersFi.close();
  qDebug("Load %d performers succeed", st.size());
  return st;
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
    if (not dict.contains("Performers")) {
      continue;
    }
    const QVariant& v = dict["Performers"];
    for (const QString& performer : v.toStringList()) {
      if (performer.isEmpty() or m_performers.contains(performer)) {
        continue;
      }
      m_performers.insert(performer.toLower());
    }
  }

  const int increCnt = int(m_performers.size()) - beforePerformersCnt;
  qDebug("Learn extra %d performers, now %u performers in total", increCnt, m_performers.size());

  QFile performersFi(SystemPath::PERFORMERS_TABLE_TXT);
  if (not performersFi.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug("file cannot open. learned performers will not update to %s.", performersFi.fileName().toStdString().c_str());
  }
  QStringList perfsLst(m_performers.cbegin(), m_performers.cend());
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
  using namespace JSON_RENAME_REGEX;
  QString valuableString = sentence.replace(keepComp, "");
  QString noActionString = valuableString.replace(andComp, " & ");
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

  static const auto rmvBelong = [](const QString& word) -> QString {
    QString s = word.trimmed();
    if (s.endsWith("'s")) {
      s.chop(2);
    } else if (s.endsWith("'")) {
      s.chop(1);
    }
    return s;
  };

  while (i < N) {
    if (i < N - 2) {
      const QString& w3 = words[i] + " " + words[i + 1] + " " + rmvBelong(words[i + 2]);
      if (m_performers.contains(w3.toLower())) {
        if (not performersList.contains(w3))
          performersList.append(w3);
        i += 3;
        continue;
      }
    }
    if (i < N - 1) {
      const QString& w2 = words[i] + " " + rmvBelong(words[i + 1]);
      if (m_performers.contains(w2.toLower())) {
        if (not performersList.contains(w2)) {
          performersList.append(w2);
        }
        i += 2;
        continue;
      }
    }
    const QString& w1 = rmvBelong(words[i]);
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
  auto& performersIns = PerformersManager::getIns();
  //  performersIns.LearningFromAPath("E:/115/test");
  const auto& wordsList =
      performersIns.MovieNameWordsSplit("Next Door Originals - Rivals Waiter vs Waiter - Theo Brady & Devin Franco flip-fuck BB 1080p");
  const auto& perfsList = performersIns.PeformersFilterOut(wordsList);
  qDebug() << perfsList;
  return 0;
}
#endif
