#include "JsonFileHelper.h"
#include "Tools/NameTool.h"
#include "Tools/PerformersManager.h"
#include "Tools/ProductionStudioManager.h"
#include "PublicVariable.h"

namespace JSONKey {
bool JsonKeySorter(const QPair<QString, QVariant>& l, const QPair<QString, QVariant>& r) {
  auto lftIt = JsonKeyPri.find(l.first);
  auto rhgtIt = JsonKeyPri.find(r.first);
  auto end = JsonKeyPri.cend();
  if (lftIt != end && rhgtIt != end) {
    return lftIt.value() < rhgtIt.value();
  }
  if (lftIt != end) {
    return true;
  }
  if (rhgtIt != end) {
    return false;
  }
  return l.first < r.first;
}
}

const QMap<QString, QString> JsonFileHelper::key2ValueType = {{JSONKey::Performers, "QStringList"},
                                                              {JSONKey::Tags, "QStringList"},
                                                              {JSONKey::Hot, "QIntList"},
                                                              {JSONKey::Rate, "int"}};

bool JsonFileHelper::MovieJsonDumper(const QVariantHash& dict, const QString& movieJsonItemPath) {
  auto jsonObject = QJsonObject::fromVariantHash(dict);
  QJsonDocument document;
  document.setObject(jsonObject);
  const auto& byteArray = document.toJson(QJsonDocument::JsonFormat::Indented);
  QFile jsonFile(movieJsonItemPath);
  if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    jsonFile.close();
    return false;
  }
  QTextStream out(&jsonFile);
  out.setCodec("UTF-8");
  out << byteArray;
  jsonFile.close();
  return true;
}

auto JsonFileHelper::HotSceneString2IntList(const QString& valueStr) -> QList<QVariant> {
  const QString& s = valueStr.trimmed();
  if (s.isEmpty()) {
    return {};
  }
  QList<QVariant> arr;
  for (const QString& perf : NameTool()(s)) {
    bool isOk = false;
    int hot = perf.toInt(&isOk);
    if (not isOk) {
      qDebug("Hot scene position[%s] is not a number", qPrintable(perf));
      continue;
    }
    arr.append(hot);
  }
  return arr;
}

QVariantHash JsonFileHelper::GetMovieFileJsonDict(const QString& fileAbsPath, const QString& performersListStr, const QString& productionStudio) {
  const QStringList& performersList = NameTool()(performersListStr);

  QFileInfo fi(fileAbsPath);
  QList<QVariant> hotSceneList = HotSceneString2IntList("");
  QVariantHash dict = {{JSONKey::Name, fi.fileName()},
                       {JSONKey::Performers, performersList},
                       {JSONKey::Studio, productionStudio},
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

QVariantHash JsonFileHelper::GetDefaultJsonFile(const QString& fileName, const QString& fileSz) {
  QVariantHash dict = {{JSONKey::Name, fileName},
                       {JSONKey::Performers, QStringList()},
                       {JSONKey::Studio, ""},
                       {JSONKey::Uploaded, ""},
                       {JSONKey::Tags, QStringList()},
                       {JSONKey::Rate, -1},
                       {JSONKey::Size, fileSz},
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
    if (dict.contains(JSONKey::Studio) and not dict[JSONKey::Studio].toString().isEmpty()) {
      continue;
    }
    const QString& sentence = dict.contains(JSONKey::Name) ? dict[JSONKey::Name].toString() : "";
    dict.insert(JSONKey::Studio, psm.hintStdStudioName(sentence));
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
    if (dict.contains(JSONKey::Studio)) {
      dict[JSONKey::Studio] = "";
    }
    auto itPS = dict.find("ProductionStudio");
    if (itPS != dict.cend()) {
      dict.erase(itPS);
      dict[JSONKey::Studio] = "";
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
  const QStringList& performerList = NameTool()(performers);
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
    if (dict[JSONKey::Studio].toString() == productionStudio) {
      continue;
    }
    dict[JSONKey::Studio] = productionStudio;
    succeedCnt += MovieJsonDumper(dict, jsonPath);
    ++tryConstuctCnt;
  }
  if (tryConstuctCnt != succeedCnt) {
    qDebug("%d/%d json add productionStudio set succeed", succeedCnt, tryConstuctCnt);
  }
  return succeedCnt;
}


QVariantHash JsonFileHelper::MovieJsonLoader(const QString& movieJsonItemPath) {
  QFile jsonFile(movieJsonItemPath);
  if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("[Normal] json file[%s] not found", qPrintable(movieJsonItemPath));
    return {};
  }
  QTextStream in(&jsonFile);
  in.setCodec("UTF-8");
  QString json_string = in.readAll();
  jsonFile.close();
  return JsonStr2Dict(json_string);
}

QVariantHash JsonFileHelper::JsonStr2Dict(const QString& jsonStr) {
  QJsonParseError jsonErr;
  QJsonDocument json_doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError) {
    qWarning("Error parse json string %d char(s): %s", jsonStr.size(), qPrintable(jsonErr.errorString()));
    return {};
  }
  const QJsonObject& rootObj = json_doc.object();
  return rootObj.toVariantHash();
}
