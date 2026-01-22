#include "CastManager.h"
#include "JsonRenameRegex.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "PathTool.h"
#include "JsonHelper.h"
#include "TableFields.h"
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

template class SingletonManager<CastManager, CAST_MGR_DATA_T>;

CastManager::CastManager() {
#ifndef RUNNING_UNIT_TESTS
  using namespace PathTool::FILE_REL_PATH;
  InitializeImpl(GetActorsListFilePath(), GetActorsBlackListFilePath());
#endif
}

void CastManager::InitializeImpl(const QString& path, const QString& blackPath) {
  mLocalFilePath = path;
  mLocalBlackFilePath = blackPath;
  CastSet() = ReadOutCasts();
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

CAST_MGR_DATA_T CastManager::ReadOutCasts() const {
  CAST_MGR_DATA_T blackList;
  {
    QFile blackFi{mLocalBlackFilePath};
    if (blackFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in(&blackFi);
      in.setCodec("UTF-8");
      while(!in.atEnd()) {
        blackList.insert(in.readLine(128));
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
    if (blackList.contains(name)) {
      continue;
    }
    actorsSet.insert(name);
  }
  castFi.close();
  LOG_D("%d performers read out", actorsSet.size());
  return actorsSet;
}

int CastManager::ForceReloadImpl() {
  int beforeStudioNameCnt = CastSet().size();
  CastSet() = ReadOutCasts();
  int afterStudioNameCnt = CastSet().size();
  LOG_D("%d performers added/removed", afterStudioNameCnt - beforeStudioNameCnt);
  return afterStudioNameCnt - beforeStudioNameCnt;
}

int CastManager::LearningFromAPath(const QString& path, bool* bHasWrite) {
  if (bHasWrite != nullptr) {
    *bHasWrite = false;
  }

  if (!QDir{path}.exists()) {
    LOG_W("path[%s] not exist", qPrintable(path));
    return 0;
  }
  CAST_MGR_DATA_T castsIncrement;
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
    CAST_MGR_DATA_T lowerSet;
    for (const auto& str : perfIt->toStringList()) {
      lowerSet.insert(str.toLower());
    }
    CastIncrement(castsIncrement, lowerSet);
  }
  LOG_D("Learn extra %d cast from %d valid json files", castsIncrement.size(), jsonFilesCnt);
  if (castsIncrement.isEmpty()) {
    return 0;
  }
  CastSet().unite(castsIncrement);

  int cnt = WriteIntoLocalDictionaryFiles(castsIncrement);
  if (cnt < 0) {
    return cnt;
  }
  if (bHasWrite != nullptr) {
    *bHasWrite = true;
  }
  return cnt;
}

int CastManager::CastIncrement(CAST_MGR_DATA_T& increments, CAST_MGR_DATA_T delta) {
  delta -= CastSet();
  increments += delta;
  CastSet() += delta;
  return delta.size();
}

int CastManager::WriteIntoLocalDictionaryFiles(const CAST_MGR_DATA_T& increments) const {
  if (increments.isEmpty()) {
    LOG_D("Empty increments, skip writing.");
    return 0;
  }
  QFile file{mLocalFilePath};
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

QStringList CastManager::FilterPerformersOut(const QStringList& words) const {
  if (words.isEmpty()) {
    return {};
  }
  const CAST_MGR_DATA_T& actorsSet = CastSet();
  QStringList performersList;
  int i = 0;
  const int N = words.size();
  while (i < N) {
    if (i < N - 2) {
      const QString& w3 = words[i] + " " + words[i + 1] + " " + RmvBelongLetter(words[i + 2]);
      if (actorsSet.contains(w3.toLower())) {
        if (!performersList.contains(w3))
          performersList.append(w3);
        i += 3;
        continue;
      }
    }
    if (i < N - 1) {
      const QString& w2 = words[i] + " " + RmvBelongLetter(words[i + 1]);
      if (actorsSet.contains(w2.toLower())) {
        if (!performersList.contains(w2)) {
          performersList.append(w2);
        }
        i += 2;
        continue;
      }
    }
    const QString& w1 = RmvBelongLetter(words[i]);
    if (!w1.isEmpty() && actorsSet.contains(w1.toLower())) {
      if (!performersList.contains(w1)) {
        performersList.append(w1);
      }
      i += 1;
      continue;
    }
    ++i;
  }
  performersList.removeDuplicates();
  return performersList;
}

QStringList CastManager::operator()(const QString& sentence) const {
  return FilterPerformersOut(SplitSentence(sentence));
}
