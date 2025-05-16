#include "CastManager.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"
#include "Tools/Json/JsonHelper.h"
#include "Tools/FileDescriptor/TableFields.h"
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

const QRegularExpression CastManager::EFFECTIVE_CAST_NAME{R"([@ _])"};
constexpr int CastManager::EFFECTIVE_CAST_NAME_LEN;

QString CastManager::GetLocalFilePath(const QString& localFilePath)  //
{
  if (!localFilePath.isEmpty()) {
    /* only used in LLT test */
    return localFilePath;
  }

#ifdef _WIN32
  const QString& perfFilePath = PreferenceSettings().value(MemoryKey::WIN32_PERFORMERS_TABLE.name).toString();
#else
  const QString& perfFilePath = PreferenceSettings().value(MemoryKey::LINUX_PERFORMERS_TABLE.name).toString();
#endif
  return perfFilePath;
}

CastManager::CastManager(const QString& localFilePath)  //
    : mLocalFilePath{GetLocalFilePath(localFilePath)}   //
{
  ForceReloadCast();
}

CastManager& CastManager::getIns() {
  qDebug("CastManager::getIns()");
  static CastManager ins;
  return ins;
}

QSet<QString> CastManager::ReadOutPerformers() const {
  QFile castFi{mLocalFilePath};
  if (!castFi.exists()) {
    qDebug("Cast list file[%s] not exist", qPrintable(castFi.fileName()));
    return {};
  }
  if (castFi.size() <= 0) {
    qDebug("Cast list file[%s] is empty", qPrintable(castFi.fileName()));
    return {};
  }
  if (!castFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("File[%s] open for read failed", qPrintable(castFi.fileName()));
    return {};
  }

  QTextStream stream(&castFi);
  stream.setCodec("UTF-8");

  decltype(m_performers) perfSet;
  QString name;
  while (!stream.atEnd()) {
    name = stream.readLine().toLower();
    // at least 2 words, or 12 char
    if (name.size() >= EFFECTIVE_CAST_NAME_LEN || name.contains(EFFECTIVE_CAST_NAME)) {
      perfSet.insert(name);
    }
  }
  castFi.close();
  qDebug("%d performers read out", perfSet.size());
  return perfSet;
}

int CastManager::ForceReloadCast() {
  int beforeStudioNameCnt = m_performers.size();
  m_performers = CastManager::ReadOutPerformers();
  int afterStudioNameCnt = m_performers.size();
  qDebug("%d performers added/removed", afterStudioNameCnt - beforeStudioNameCnt);
  return afterStudioNameCnt - beforeStudioNameCnt;
}

int CastManager::LearningFromAPath(const QString& path, bool* bHasWrite) {
  if (bHasWrite != nullptr) {
    *bHasWrite = false;
  }

  if (!QDir{path}.exists()) {
    qWarning("path[%s] not exist", qPrintable(path));
    return 0;
  }
  decltype(m_performers) castsIncrement;
  QDirIterator it{path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  int jsonFilesCnt{0};
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = JsonHelper::MovieJsonLoader(jsonPath);
    auto perfIt = dict.constFind(ENUM_2_STR(Cast));
    if (perfIt == dict.cend()) {
      continue;
    }
    ++jsonFilesCnt;
    const QVariant& v = perfIt.value();
    QString lowercaseCast;
    for (const QString& performer : v.toStringList()) {
      lowercaseCast = performer.trimmed().toLower();
      if (lowercaseCast.isEmpty() || m_performers.contains(lowercaseCast)) {
        continue;
      }
      castsIncrement.insert(lowercaseCast);
    }
  }
  qDebug("Learn extra %d cast from %d valid json files", castsIncrement.size(), jsonFilesCnt);
  if (castsIncrement.isEmpty()) {
    return 0;
  }
  m_performers.unite(castsIncrement);

  QFile performersFi{mLocalFilePath};
  if (!performersFi.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
    qWarning("Open file[%s] to write failed. Cannot write studio increasement.", qPrintable(performersFi.fileName()));
    return -1;
  }
  QTextStream stream{&performersFi};
  stream.setCodec("UTF-8");
  for (const QString& perf : castsIncrement) {
    stream << perf << '\n';
  }
  stream.flush();
  performersFi.close();
  if (bHasWrite != nullptr) {
    *bHasWrite = true;
  }
  return castsIncrement.size();
}

QStringList CastManager::SplitSentence(QString sentence) {
  if (sentence.isEmpty()) {
    return {};
  }
  using namespace JSON_RENAME_REGEX;
  sentence.replace(DISCRAD_LETTER_COMP, " ");
  sentence.replace(AND_COMP, " ");
  sentence.remove(RESOLUTION_COMP);
  return sentence.split(AT_LEAST_1_SPACE_COMP);
}

QString CastManager::RmvBelongLetter(const QString& word) {
  QString s = word.trimmed();
  if (s.endsWith("'s")) {
    s.chop(2);
  } else if (s.endsWith("'")) {
    s.chop(1);
  }
  return s;
}

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
        if (!performersList.contains(w3))
          performersList.append(w3);
        i += 3;
        continue;
      }
    }
    if (i < N - 1) {
      const QString& w2 = words[i] + " " + RmvBelongLetter(words[i + 1]);
      if (m_performers.contains(w2.toLower())) {
        if (!performersList.contains(w2)) {
          performersList.append(w2);
        }
        i += 2;
        continue;
      }
    }
    const QString& w1 = RmvBelongLetter(words[i]);
    if (!w1.isEmpty() && m_performers.contains(w1.toLower())) {
      if (!performersList.contains(w1))
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
