#include "StudiosManager.h"

#include "PathTool.h"
#include "PublicVariable.h"
#include "JsonRenameRegex.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "JsonKey.h"
#include "JsonHelper.h"
#include <QVariantHash>
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

template class SingletonManager<StudiosManager, STUDIO_MGR_DATA_T>;

constexpr int StudiosManager::STUDIO_HYPEN_MAX_INDEX;
bool StudiosManager::isHypenIndexValid(const QString& sentence, int& hypenIndex) {
  hypenIndex = sentence.indexOf('-');
  return hypenIndex > 0 && (hypenIndex <= STUDIO_HYPEN_MAX_INDEX);
}

bool StudiosManager::isHypenIndexValidReverse(const QString& sentence, int& hypenIndex) {
  hypenIndex = sentence.lastIndexOf('-');
  // sentence.size() - hypenIndex <= STUDIO_HYPEN_MAX_INDEX
  return (hypenIndex < sentence.size() - 1) && (hypenIndex >= sentence.size() - STUDIO_HYPEN_MAX_INDEX);
}

StudiosManager::StudiosManager() {
#ifndef RUNNING_UNIT_TESTS
  using namespace PathTool::FILE_REL_PATH;
  InitializeImpl(GetStudiosListFilePath(), GetActorsBlackListFilePath());
#endif
}

void StudiosManager::InitializeImpl(const QString& path, const QString& blackPath) {
  mLocalFilePath = path;
  mLocalBlackFilePath = blackPath;
  ProStudioMap() = ReadOutStdStudioName();
}

STUDIO_MGR_DATA_T StudiosManager::ReadOutStdStudioName() const {
  QSet<QString> blackList;
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

  QFile studioFi{mLocalFilePath};
  if (!studioFi.exists()) {
    LOG_D("Studio list file[%s] not exist", qPrintable(studioFi.fileName()));
    return {};
  }
  if (studioFi.size() <= 0) {
    LOG_D("Studio list file[%s] is empty", qPrintable(studioFi.fileName()));
    return {};
  }
  if (!studioFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_D("File[%s] open for read failed", qPrintable(studioFi.fileName()));
    return {};
  }
  STUDIO_MGR_DATA_T stdStudioNameDict;

  QTextStream in(&studioFi);
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
      LOG_W("The %dth line of file[%s] is invalid", lineIndex, qPrintable(studioFi.fileName()));
      continue;
    }
    QString befName = line.left(tabKeyInd);
    if (blackList.contains(befName)) {
      continue;
    }
    QString aftName = line.mid(tabKeyInd + 1);
    if (blackList.contains(aftName)) {
      continue;
    }
    stdStudioNameDict[befName] = aftName;
  }
  studioFi.close();
  LOG_D("%d studio item(s) read out from %d lines", stdStudioNameDict.size(), lineIndex);
  return stdStudioNameDict;
}

int StudiosManager::ForceReloadImpl() {
  int befCnt = ProStudioMap().size();

  STUDIO_MGR_DATA_T newStudioNames = StudiosManager::ReadOutStdStudioName();
  ProStudioMap().swap(newStudioNames);

  int aftCnt = ProStudioMap().size();
  LOG_D("standard studio names rule from %d to %d", befCnt, aftCnt);
  return aftCnt - befCnt;
}

int StudiosManager::LearningFromAPath(const QString& path, bool* bHasWrite) {
  if (bHasWrite != nullptr) {
    *bHasWrite = false;
  }
  using namespace JsonKey;
  if (!QDir{path}.exists()) {
    LOG_D("path[%s] not exist", qPrintable(path));
    return 0;
  }

  STUDIO_MGR_DATA_T studiosIncrementMap;
  QDirIterator it{path, {"*.json"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  int jsonFilesCnt{0};
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    const QVariantHash& dict = JsonHelper::MovieJsonLoader(jsonPath);
    auto studioIt = dict.constFind(ENUM_2_STR(Studio));
    if (studioIt == dict.cend()) {
      continue;
    }
    ++jsonFilesCnt;
    StudioIncrement(studiosIncrementMap, studioIt->toString());
  }
  LOG_D("Learn extra %d studios from %d json files", studiosIncrementMap.size(), jsonFilesCnt);
  if (studiosIncrementMap.isEmpty()) {
    return 0;
  }
  int cnt = WriteIntoLocalDictionaryFiles(studiosIncrementMap);
  if (cnt < 0) {
    return -1;
  }
  if (bHasWrite != nullptr) {
    *bHasWrite = true;
  }
  return cnt;
}

int StudiosManager::StudioIncrement(STUDIO_MGR_DATA_T& increments, const QString& newStudio) {
  int cnt{0};
  for (const QString& coarseStudioName : GetCoarseStudioNames(newStudio)) {
    auto it = ProStudioMap().find(coarseStudioName);
    if (coarseStudioName.isEmpty() || it != ProStudioMap().cend()) {
      continue;
    }
    increments[coarseStudioName] = newStudio;
    ProStudioMap()[coarseStudioName] = newStudio;
    ++cnt;
  }
  return cnt;
}

int StudiosManager::WriteIntoLocalDictionaryFiles(const STUDIO_MGR_DATA_T& increments) const {
  if (increments.isEmpty()) {
    LOG_D("Empty increments, skip writing.");
    return 0;
  }

  QFile txtFile{mLocalFilePath};
  if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
    LOG_W("Failed to open file [%s]: %s", qPrintable(txtFile.fileName()), qPrintable(txtFile.errorString()));
    return -1;
  }

  QTextStream out(&txtFile);
  out.setCodec("UTF-8");

  QString buffer;
  buffer.reserve(increments.size() * 64);
  for (auto it = increments.constBegin(); it != increments.constEnd(); ++it) {
    buffer.append(it.key()).append('\t').append(it.value()).append('\n');
  }
  out << buffer;
  if (out.status() != QTextStream::Ok) {
    LOG_W("Write error occurred: %d", out.status());
    return -2;
  }
  txtFile.close();
  LOG_D("Successfully wrote %d studio items to file %s", increments.size(), qPrintable(mLocalFilePath));
  return increments.size();
}

using namespace JSON_RENAME_REGEX;

QSet<QString> StudiosManager::GetCoarseStudioNames(QString standardPs) const {
  // input: "RealMadridFC"
  // output:
  // realmadridfc  =>  pslower
  // real madrid fc => psWithSpaceLower
  const QString& pslower = standardPs.toLower();
  const QString& psWithSpaceLower = standardPs                                         //
                                        .replace(SPLIT_BY_UPPERCASE_COMP1, "\\1 \\2")  //
                                        .replace(SPLIT_BY_UPPERCASE_COMP2, "\\1 \\2")  //
                                        .toLower();                                    //
  return {pslower, psWithSpaceLower};
}

QString StudiosManager::FileName2StudioNameSection(QString sentence) const {
  sentence.remove(TORRENT_LEADING_STR_COMP);   // remove [FFL], [FL], [GT]
  sentence.remove(LEADING_OPEN_BRACKET_COMP);  // remove open braces [({
  sentence.replace(NON_LEADING_BRACKET_COMP, "-");
  sentence = sentence.trimmed();
  int hypenIndex{-1};
  if (!isHypenIndexValid(sentence, hypenIndex)) {
    return "";
  }
  QString studioSection = sentence.left(hypenIndex);
  studioSection.remove(DISCRAD_LETTER_COMP);
  return studioSection.trimmed();
}

QString StudiosManager::FileNameLastSection2StudioNameSection(QString sentence) const {
  static const QRegularExpression DVD_PATTERN{R"( DVD$)", QRegularExpression::CaseInsensitiveOption | QRegularExpression::OptimizeOnFirstUsageOption};
  sentence.remove(DVD_PATTERN);
  int hypenIndex{-1};
  if (!isHypenIndexValidReverse(sentence, hypenIndex)) {
    return "";
  }
  QString studioSection = sentence.mid(hypenIndex + 1);
  studioSection.remove(DISCRAD_LETTER_COMP);
  return studioSection.trimmed();
}

auto StudiosManager::operator()(const QString& sentence) const -> QString {
  const QString studioNameFrontSection = FileName2StudioNameSection(sentence);
  auto itFront = ProStudioMap().find(studioNameFrontSection.toLower());
  if (itFront != ProStudioMap().cend()) {
    return itFront.value();
  }
  const QString studioNameBackSection = FileNameLastSection2StudioNameSection(sentence);
  auto itBack = ProStudioMap().find(studioNameBackSection.toLower());
  if (itBack != ProStudioMap().cend()) {
    return itBack.value();
  }
  return ""; // not in table
}
