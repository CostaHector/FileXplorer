#include "CastManager.h"
#include "JsonRenameRegex.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "PathTool.h"
#include "JsonKey.h"
#include "JsonHelper.h"
#include "TableFields.h"
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

template class SingletonManager<CastManager, CAST_MGR_DATA_T>;

CastManager::CastManager() {
#ifndef RUNNING_UNIT_TESTS
  using namespace PathTool::FILE_REL_PATH;
  InitializeImpl(GetActorsListFilePath(), GetMononymActorsListFilePath());
#endif
}

void CastManager::InitializeImpl(const QString& path, const QString& blackPath) {
  mLocalFilePath = path;
  mLocalSingleWordFilePath = blackPath;
  std::tie(CastSet(), mSingleWordActors) = ReadOutActors();
}

bool CastManager::IsActorNameValid(const QString& actorName) {
  static constexpr int EFFECTIVE_ACTOR_NAME_IF_LENGTH_GE = 12;
  if (actorName.size() >= EFFECTIVE_ACTOR_NAME_IF_LENGTH_GE) {
    return true;
  }
  static const QRegularExpression EFFECTIVE_CAST_NAME_CONTAIN_LETTERS{R"([@ _\.0-9])"};
  if (actorName.contains(EFFECTIVE_CAST_NAME_CONTAIN_LETTERS)) {
    return true;
  }
  return false;
}

std::pair<CAST_MGR_DATA_T, CAST_MGR_DATA_T> CastManager::ReadOutActors() const {
  CAST_MGR_DATA_T singleWordActors;
  {
    QFile singleWordFi{mLocalSingleWordFilePath};
    if (singleWordFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in(&singleWordFi);
      in.setCodec("UTF-8");
      while (!in.atEnd()) {
        singleWordActors.insert(in.readLine(128));
      }
    }
  }

  QFile castFi{mLocalFilePath};
  if (!castFi.exists()) {
    LOG_D("Cast list file[%s] not exist", qPrintable(castFi.fileName()));
    return {};
  }
  if (castFi.size() <= 0) {
    LOG_D("Cast list file[%s] is empty", qPrintable(castFi.fileName()));
    return {};
  }
  if (!castFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_D("File[%s] open for read failed", qPrintable(castFi.fileName()));
    return {};
  }

  QTextStream stream(&castFi);
  stream.setCodec("UTF-8");

  CAST_MGR_DATA_T actorsSet;
  QString name;
  while (!stream.atEnd()) {
    name = stream.readLine().toLower();
    if (!IsActorNameValid(name)) {
      continue;
    }
    if (singleWordActors.contains(name)) {
      continue;
    }
    actorsSet.insert(name);
  }
  castFi.close();
  LOG_D("%d performers read out", actorsSet.size());
  return {actorsSet, singleWordActors};
}

int CastManager::ForceReloadImpl() {
  int beforeStudioNameCnt = CastSet().size();
  std::tie(CastSet(), mSingleWordActors) = ReadOutActors();
  int afterStudioNameCnt = CastSet().size();
  LOG_D("%d performers added/removed", afterStudioNameCnt - beforeStudioNameCnt);
  return afterStudioNameCnt - beforeStudioNameCnt;
}

CAST_MGR_DATA_T CastManager::ActorIncrement(const CAST_MGR_DATA_T& actors) {
  CAST_MGR_DATA_T increments = actors;
  increments -= CastSet();
  CastSet() += increments;
  return increments;
}

CAST_MGR_DATA_T CastManager::SingleWordActorIncrement(const CAST_MGR_DATA_T& singleWordActors) {
  CAST_MGR_DATA_T increments = singleWordActors;
  increments -= mSingleWordActors;
  mSingleWordActors += increments;
  return increments;
}

int CastManager::WriteIntoLocalDictionaryFiles(const CAST_MGR_DATA_T& increments, bool bWriteIntoSingleWordFilePath) const {
  if (increments.isEmpty()) {
    LOG_D("Empty increments, skip writing.");
    return 0;
  }
  QFile file{bWriteIntoSingleWordFilePath ? mLocalSingleWordFilePath : mLocalFilePath};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
    LOG_W("Open file[%s] to write failed. Cannot write studio increasement.", qPrintable(file.fileName()));
    return -1;
  }
  QTextStream out(&file);
  out.setCodec("UTF-8");

  QString content;
  content.reserve(increments.size() * 32);
  for (const QString& item : increments) {
    content.append(item).append('\n');
  }
  out << content;
  if (out.status() != QTextStream::Ok) {
    LOG_C("Write failed: %s", qPrintable(file.errorString()));
    return -2;
  }
  file.close();
  LOG_D("Successfully wrote %d cast items to file %s", increments.size(), qPrintable(mLocalFilePath));
  return increments.size();
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

QStringList CastManager::FilterPerformersOut(const QStringList& words, const bool bIsActorFromSingleWordStudio) const {
  if (words.isEmpty()) {
    return {};
  }
  const CAST_MGR_DATA_T& actorsSet = CastSet();
  QStringList actorsList;
  int i = 0;
  const int N = words.size();
  while (i < N) {
    if (i < N - 2) {
      const QString& w3 = words[i] + " " + words[i + 1] + " " + RmvBelongLetter(words[i + 2]);
      if (actorsSet.contains(w3.toLower())) {
        actorsList.append(w3);
        i += 3;
        continue;
      }
    }
    if (i < N - 1) {
      const QString& w2 = words[i] + " " + RmvBelongLetter(words[i + 1]);
      if (actorsSet.contains(w2.toLower())) {
        actorsList.append(w2);
        i += 2;
        continue;
      }
    }
    const QString& w1 = RmvBelongLetter(words[i]);
    if (!w1.isEmpty() && (actorsSet.contains(w1.toLower())                                               //
                          || (bIsActorFromSingleWordStudio && mSingleWordActors.contains(w1.toLower()))  //
                          )                                                                              //
    ) {
      actorsList.append(w1);
      i += 1;
      continue;
    }
    ++i;
  }
  std::sort(actorsList.begin(), actorsList.end());
  actorsList.erase(std::unique(actorsList.begin(), actorsList.end()), actorsList.end());
  return actorsList;
}

QStringList CastManager::operator()(const QString& sentence, const bool bIsActorFromSingleWordStudio) const {
  return FilterPerformersOut(SplitSentence(sentence), bIsActorFromSingleWordStudio);
}
