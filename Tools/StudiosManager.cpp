#include "StudiosManager.h"

#include "PathTool.h"
#include "PublicVariable.h"
#include "PublicMacro.h"
#include "MemoryKey.h"
#include "JsonKey.h"
#include "JsonHelper.h"
#include <QVariantHash>
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

constexpr int StudiosManager::STUDIO_HYPEN_MAX_INDEX;
bool StudiosManager::isHypenIndexValid(const QString& sentence, int& hypenIndex) {
  hypenIndex = sentence.indexOf('-');
  return hypenIndex > 0 && (hypenIndex <= STUDIO_HYPEN_MAX_INDEX);
}

bool StudiosManager::isHypenIndexValidReverse(const QString& sentence, int& hypenIndex){
  hypenIndex = sentence.lastIndexOf('-');
  // sentence.size() - hypenIndex <= STUDIO_HYPEN_MAX_INDEX
  return (hypenIndex < sentence.size() - 1) && (hypenIndex >= sentence.size() - STUDIO_HYPEN_MAX_INDEX);
}

QString StudiosManager::GetLocalFilePath(const QString& localFilePath) {
  if (!localFilePath.isEmpty()) { /* only used in LLT test */
    return localFilePath;
  }
  using namespace PathTool::FILE_REL_PATH;
  static const QString stdStudiosFilePath = PathTool::GetPathByApplicationDirPath(STANDARD_STUDIO_NAME);
  return stdStudiosFilePath;
}

StudiosManager& StudiosManager::getIns() {
  qDebug("StudiosManager::getIns()");
  static StudiosManager ins;
  return ins;
}

StudiosManager::StudiosManager(const QString& localFilePath)  //
  : mLocalFilePath{GetLocalFilePath(localFilePath)}         //
{
  ForceReloadStudio();
}

QHash<QString, QString> StudiosManager::ReadOutStdStudioName() const {
  QFile studioFi{mLocalFilePath};
  if (!studioFi.exists()) {
    qDebug("Studio list file[%s] not exist", qPrintable(studioFi.fileName()));
    return {};
  }
  if (studioFi.size() <= 0) {
    qDebug("Studio list file[%s] is empty", qPrintable(studioFi.fileName()));
    return {};
  }
  if (!studioFi.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("File[%s] open for read failed", qPrintable(studioFi.fileName()));
    return {};
  }
  QHash<QString, QString> stdStudioNameDict;

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
      qWarning("The %dth line of file[%s] is invalid", lineIndex, qPrintable(studioFi.fileName()));
      continue;
    }
    stdStudioNameDict[line.left(tabKeyInd)] = line.mid(tabKeyInd + 1);
  }
  studioFi.close();
  qDebug("%d studio item(s) read out from %d lines", stdStudioNameDict.size(), lineIndex);
  return stdStudioNameDict;
}

int StudiosManager::ForceReloadStudio() {
  int befCnt = m_prodStudioMap.size();
  auto newStudioNames = StudiosManager::ReadOutStdStudioName();
  m_prodStudioMap.swap(newStudioNames);
  int aftCnt = m_prodStudioMap.size();
  qDebug("standard studio names rule from %d to %d", befCnt, aftCnt);
  return aftCnt - befCnt;
}

int StudiosManager::LearningFromAPath(const QString& path, bool* bHasWrite) {
  if (bHasWrite != nullptr) {
    *bHasWrite = false;
  }
  using namespace JsonKey;
  if (!QDir{path}.exists()) {
    qDebug("path[%s] not exist", qPrintable(path));
    return 0;
  }

  decltype(m_prodStudioMap) studiosIncrementMap;
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
  qDebug("Learn extra %d studios from %d json files", studiosIncrementMap.size(), jsonFilesCnt);
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

int StudiosManager::StudioIncrement(QHash<QString, QString>& increments, const QString& newStudio) {
  int cnt{0};
  for (const QString& coarseStudioName : GetCoarseStudioNames(newStudio)) {
    auto it = m_prodStudioMap.find(coarseStudioName);
    if (coarseStudioName.isEmpty() || it != m_prodStudioMap.cend()) {
      continue;
    }
    increments[coarseStudioName] = newStudio;
    m_prodStudioMap[coarseStudioName] = newStudio;
    ++cnt;
  }
  return cnt;
}

int StudiosManager::WriteIntoLocalDictionaryFiles(const QHash<QString, QString>& increments) const {
  if (increments.isEmpty()) {
    qDebug("Empty increments, skip writing.");
    return 0;
  }

  QFile txtFile{mLocalFilePath};
  if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
    qWarning("Failed to open file [%s]: %s", qPrintable(txtFile.fileName()), qPrintable(txtFile.errorString()));
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
    qWarning("Write error occurred: %d", out.status());
    return -2;
  }
  txtFile.close();
  qDebug("Successfully wrote %d studio items to file %s", increments.size(), qPrintable(mLocalFilePath));
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
  auto itFront = m_prodStudioMap.find(studioNameFrontSection.toLower());
  if (itFront != m_prodStudioMap.cend()) {
    return itFront.value();
  }
  const QString studioNameBackSection = FileNameLastSection2StudioNameSection(sentence);
  auto itBack = m_prodStudioMap.find(studioNameBackSection.toLower());
  if (itBack != m_prodStudioMap.cend()) {
    return itBack.value();
  }
  return studioNameFrontSection;
}
