#include "JsonFileHelper.h"
#include "Tools/ProductionStudioManager.h"
#include "Tools/PerformersManager.h"

const QMap<QString, QString> JsonFileHelper::key2ValueType = {{JSONKey::Performers, "QStringList"},
                                                              {JSONKey::Tags, "QStringList"},
                                                              {JSONKey::Hot, "QIntList"},
                                                              {JSONKey::Rate, "int"}};

QVariantHash JsonFileHelper::GetMovieFileJsonDict(const QString& fileAbsPath, const QString& performersListStr, const QString& productionStudio) {
  QStringList performersList;
  const QString& ps = performersListStr.trimmed();
  if (not ps.isEmpty()) {
    performersList = ps.split(JSON_RENAME_REGEX::SEPERATOR_COMP);
  }

  QFileInfo fi(fileAbsPath);
  QList<QVariant> hotSceneList = HotSceneString2IntList("");
  QVariantHash dict = {{JSONKey::Name, fi.fileName()},
                       {JSONKey::Performers, performersList},
                       {JSONKey::ProductionStudio, productionStudio},
                       {JSONKey::Uploaded, fi.birthTime().toString("yyyyMMdd")},
                       {JSONKey::Tags, QStringList()},
                       {JSONKey::Rate, -1},
                       {JSONKey::Size, QString::number(fi.size())},
                       {JSONKey::Resolution, ""},
                       {JSONKey::Bitrate, ""},
                       {JSONKey::Hot, hotSceneList},
                       {JSONKey::Detail, ""}};
  return dict;
}

QVariantHash JsonFileHelper::GetDefaultJsonFile(const QString& fileName) {
  QVariantHash dict = {{JSONKey::Name, fileName},
                       {JSONKey::Performers, QStringList()},
                       {JSONKey::ProductionStudio, ""},
                       {JSONKey::Uploaded, ""},
                       {JSONKey::Tags, QStringList()},
                       {JSONKey::Rate, -1},
                       {JSONKey::Size, QString::number(0)},
                       {JSONKey::Resolution, ""},
                       {JSONKey::Bitrate, ""},
                       {JSONKey::Hot, QList<QVariant>()},
                       {JSONKey::Detail, ""}};
  return dict;
}

int JsonFileHelper::ConstructJsonForVids(const QString& path, const QString& productionStudio, const QString& performersListStr) {
  if (not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;
  QDirIterator it(path, TYPE_FILTER::VIDEO_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& vidPath = it.filePath();
    const QString& jsonPath = GetJsonFilePath(vidPath);
    if (QFile::exists(jsonPath)) {
      continue;
    }
    const auto& dict = GetMovieFileJsonDict(vidPath, performersListStr, productionStudio);
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json contructed succeed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}

int JsonFileHelper::JsonPerformersKeyValuePairAdd(const QString& path) {
  if (not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryKVPairCnt = 0;
  
  static PerformersManager& pm = PerformersManager::getIns();

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (dict.contains(JSONKey::Performers) and not dict[JSONKey::Performers].toStringList().isEmpty()) {
      continue;
    }

    const QString& sentence = dict.contains(JSONKey::Name) ? dict[JSONKey::Name].toString() : "";
    QStringList perfL = pm(sentence);
    perfL.removeDuplicates();
    dict.insert(JSONKey::Performers, perfL);

    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryKVPairCnt;
  }
  if (tryKVPairCnt != succeedCnt) {
    qDebug("%d/%d json add performer key-value pair succeed", succeedCnt, tryKVPairCnt);
  }
  return succeedCnt;
}

int JsonFileHelper::JsonProductionStudiosKeyValuePairAdd(const QString& path) {
  if (not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryKVPairCnt = 0;

  static ProductionStudioManager& psm = ProductionStudioManager::getIns();

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (dict.contains(JSONKey::ProductionStudio) and not dict[JSONKey::ProductionStudio].toString().isEmpty()) {
      continue;
    }
    const QString& sentence = dict.contains(JSONKey::Name) ? dict[JSONKey::Name].toString() : "";
    dict.insert(JSONKey::ProductionStudio, psm(sentence));
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryKVPairCnt;
  }
  if (tryKVPairCnt != succeedCnt) {
    qDebug("%d/%d json add production studio key-value pair succeed", succeedCnt, tryKVPairCnt);
  }
  return succeedCnt;
}

int JsonFileHelper::JsonValuePerformersProductionStudiosCleaner(const QString& path) {
  if (not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryCleanCnt = 0;
  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (dict.contains(JSONKey::Performers)) {
      dict[JSONKey::Performers] = QStringList();
    }
    if (dict.contains(JSONKey::ProductionStudio)) {
      dict[JSONKey::ProductionStudio] = "";
    }
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryCleanCnt;
  }
  if (tryCleanCnt != succeedCnt) {
    qDebug("%d/%d json clean performer or production studio name succeed", succeedCnt, tryCleanCnt);
  }
  return succeedCnt;
}

int JsonFileHelper::JsonValuePerformersAdder(const QString& path, const QString& performers) {
  const QStringList& performerList = performers.trimmed().split(JSON_RENAME_REGEX::SEPERATOR_COMP);
  if (performerList.isEmpty() or not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    QStringList perfL = dict[JSONKey::Performers].toStringList() + performerList;
    perfL.removeDuplicates();
    dict[JSONKey::Performers] = perfL;
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json add performer succeed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}

int JsonFileHelper::JsonValueProductionStudioSetter(const QString& path, const QString& _productionStudio) {
  const QString& productionStudio = _productionStudio.trimmed();
  if (not QFileInfo(path).isDir()) {
    return -1;
  }
  int succeedCnt = 0;
  int tryConstuctCnt = 0;

  QDirIterator it(path, TYPE_FILTER::JSON_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QString& jsonPath = it.filePath();
    QVariantHash dict = MovieJsonLoader(jsonPath);
    if (dict[JSONKey::ProductionStudio].toString() == productionStudio) {
      continue;
    }
    dict[JSONKey::ProductionStudio] = productionStudio;
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json add productionStudio set succeed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}
