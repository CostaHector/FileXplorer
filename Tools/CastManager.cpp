#include "CastManager.h"
#include "Component/Notificator.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"
#include "Tools/JsonFileHelper.h"
#include "Tools/FileDescriptor/TableFields.h"
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

CastManager::CastManager()       //
    : m_performers{ReadOutPerformers()},     //
      perfsCompleter(m_performers.values())  //
{
  perfsCompleter.setCaseSensitivity(Qt::CaseInsensitive);
  perfsCompleter.setCompletionMode(QCompleter::CompletionMode::PopupCompletion);
}

CastManager& CastManager::getIns() {
  qDebug("CastManager::getIns()");
  static CastManager ins;
  return ins;
}

QSet<QString> CastManager::ReadOutPerformers() {
#ifdef _WIN32
  const QString& perfFilePath = PreferenceSettings().value(MemoryKey::WIN32_PERFORMERS_TABLE.name).toString();
#else
  const QString& perfFilePath = PreferenceSettings().value(MemoryKey::LINUX_PERFORMERS_TABLE.name).toString();
#endif

  QFile performersFi(perfFilePath);
  if (!performersFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning("file[%s] not found or open for read failed.", qPrintable(performersFi.fileName()));
    return {};
  }
  QTextStream stream(&performersFi);
  stream.setCodec("UTF-8");

  static const QRegularExpression VALID_CAST_NAME{R"([@ _])"};
  decltype(m_performers) perfSet;
  QString name;
  while (!stream.atEnd()) {
    name = stream.readLine().toLower();
    if (name.size() > 12 || name.contains(VALID_CAST_NAME)) { // at least 2 words, or 12 char
      perfSet.insert(name);
    }
  }
  performersFi.close();
  qDebug("%d performers read out", perfSet.size());
  return perfSet;
}

int CastManager::ForceReloadPerformers() {
  int beforeStudioNameCnt = m_performers.size();
  m_performers = CastManager::ReadOutPerformers();
  int afterStudioNameCnt = m_performers.size();
  qDebug("%d performers added/removed", afterStudioNameCnt - beforeStudioNameCnt);
  return afterStudioNameCnt - beforeStudioNameCnt;
}

int CastManager::LearningFromAPath(const QString& path) {
  if (!QDir(path).exists()) {
    return 0;
  }
  decltype(m_performers) castsIncrement;
  QDirIterator it(path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = JsonFileHelper::MovieJsonLoader(jsonPath);
    auto perfIt = dict.find(ENUM_TO_STRING(Cast));
    if (perfIt != dict.cend()) {
      continue;
    }
    const QVariant& v = perfIt.value();
    for (const QString& performer : v.toStringList()) {
      if (performer.isEmpty() || m_performers.contains(performer)) {
        continue;
      }
      castsIncrement.insert(performer.toLower());
    }
  }
  qDebug("Learn extra %d performers from json files", castsIncrement.size());
  if (castsIncrement.isEmpty()) {
    return castsIncrement.size();
  }

#ifdef _WIN32
  const QString perfsFilePath = PreferenceSettings().value(MemoryKey::WIN32_PERFORMERS_TABLE.name).toString();
#else
  const QString perfsFilePath = PreferenceSettings().value(MemoryKey::LINUX_PERFORMERS_TABLE.name).toString();
#endif
  QFile performersFi{perfsFilePath};
  if (!performersFi.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
    qWarning("Open [%s] to write failed. Cast will not update.", qPrintable(perfsFilePath));
    Notificator::warning("Open [%s] to write failed. Cast will not update.", perfsFilePath);
    return -1;
  }
  QTextStream stream(&performersFi);
  stream.setCodec("UTF-8");
  for (const QString& perf : castsIncrement) {
    stream << perf << '\n';
  }
  stream.flush();
  performersFi.close();
  return castsIncrement.size();
}

QStringList CastManager::SplitSentence(QString sentence) {
  if (sentence.isEmpty()) {
    return {};
  }
  using namespace JSON_RENAME_REGEX;
  sentence.replace(DISCRAD_LETTER_COMP, " ");
  sentence.replace(AND_COMP, " & ");
  sentence.remove(RESOLUTION_COMP);
  return sentence.split(AT_LEAST_1_SPACE_COMP);
}

auto CastManager::RmvBelongLetter(const QString& word) -> QString {
  QString s = word.trimmed();
  if (s.endsWith("'s")) {
    s.chop(2);
  } else if (s.endsWith("'")) {
    s.chop(1);
  }
  return s;
};

QStringList CastManager::FilterPerformersOut(const QStringList& words) const {
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

QStringList CastManager::operator()(const QString& sentence) const  //
{
  return FilterPerformersOut(SplitSentence(sentence));
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
