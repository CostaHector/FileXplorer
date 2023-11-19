#ifndef JSONFILEHELPER_H
#define JSONFILEHELPER_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include "Component/PerformersManager.h"
#include "Component/ProductionStudioManager.h"
#include "PublicVariable.h"

#include <QDirIterator>
#include <QFile>
const QRegExp SEPERATOR_COMP(" and | & | , |,\r\n|, | ,|& | &|; | ;|\r\n|,\n|\n|,|;|&", Qt::CaseInsensitive);

class JsonFileHelper {
 public:
  JsonFileHelper();
  static auto MovieJsonDumper(const QVariantHash& dict, const QString& movieJsonItemPath) -> bool {
    auto jsonObject = QJsonObject::fromVariantHash(dict);
    QJsonDocument document;
    document.setObject(jsonObject);
    const auto& byteArray = document.toJson(QJsonDocument::JsonFormat::Indented);
    QFile jsonFile(movieJsonItemPath);
    if (not jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      jsonFile.close();
      return false;
    }
    QTextStream out(&jsonFile);
    out.setCodec("UTF-8");
    out << byteArray;
    jsonFile.close();
    return true;
  }

  static auto MovieJsonLoader(const QString& movieJsonItemPath) -> QVariantHash {
    QFile jsonFile(movieJsonItemPath);
    QString json_string;
    if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in(&jsonFile);
      in.setCodec("UTF-8");
      json_string = in.readAll();
      jsonFile.close();
    } else {
      qDebug("[Normal] json file not found");
      return {};
    }
    QJsonParseError jsonErr;
    QJsonDocument json_doc = QJsonDocument::fromJson(json_string.toUtf8(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError) {
      qDebug("Error when parse");
      return {};
    }
    const QJsonObject& rootObj = json_doc.object();
    return rootObj.toVariantHash();
  }

  static auto MapToOrderedList(const QVariantHash& in) -> QList<QPair<QString, QVariant>> {
    QList<QPair<QString, QVariant>> out;
    for (auto it = in.cbegin(); it != in.cend(); ++it) {
      out.append(qMakePair(it.key(), it.value()));
    }
    std::sort(out.begin(), out.end(), JSONKey::KeySorter);
    return out;
  }

  static auto PerformersString2StringList(const QString& valueStr) -> QStringList {
    if (valueStr.isEmpty()) {
      return {};
    }
    QStringList arr;
    for (const QString& perfRaw : valueStr.split(SEPERATOR_COMP)) {
      const QString& perf = perfRaw.trimmed();
      if (not arr.contains(perf)) {
        arr << perf;
      }
    }
    return arr;
  }

  static auto HotSceneString2IntList(const QString& valueStr) -> QList<QVariant> {
    const QString& s = valueStr.trimmed();
    if (s.isEmpty()) {
      return {};
    }
    decltype(HotSceneString2IntList("")) arr;
    for (const QString& perfRaw : s.split(SEPERATOR_COMP)) {
      const QString& perf = perfRaw.trimmed();
      bool isOk = false;
      int hot = perf.toInt(&isOk);
      if (not isOk) {
        qDebug("Hot scene position[%s] is not a number", perf.toStdString().c_str());
        continue;
      }
      arr.append(hot);
    }
    return arr;
  }

  static auto GetJsonValueString(const QString& keyName, const QVariant& v) -> QString {
    QString valueStr;
    if (key2ValueType.contains(keyName)) {
      if (key2ValueType[keyName] == "QStringList") {
        valueStr = v.toStringList().join(", ");
      } else if (key2ValueType[keyName] == "QIntList") {
        QStringList hotSceneSL;
        for (QVariant ivariant : v.toList()) {
          bool isInt = false;
          int hot = ivariant.toInt(&isInt);
          if (not isInt) {
            continue;
          }
          hotSceneSL.append(QString::number(hot));
        }
        valueStr = hotSceneSL.join(", ");
      } else if (key2ValueType[keyName] == "int") {
        valueStr = QString::number(v.toInt());
      } else {
        qDebug("type(map[%s]) cannot be processed", keyName.toStdString().c_str());
      }
    } else {
      valueStr = v.toString();
    }
    return valueStr;
  }

  static QVariantHash GetInitJsonFile(const QString& fileAbsPath, const QString& performersListStr = "", const QString& productionStudio = "") {
    QStringList performersList;
    const QString& ps = performersListStr.trimmed();
    if (not ps.isEmpty()) {
      performersList = ps.split(SEPERATOR_COMP);
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

  static QString GetJsonFilePath(const QString& vidsPath) {
    const int sufLen = vidsPath.lastIndexOf('.');
    const QString& jsonPath = vidsPath.left(sufLen) + ".json";
    return jsonPath;
  }

  static int ConstructJsonForVids(const QString& path, const QString& productionStudio = "", const QString& performersListStr = "") {
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
      const auto& dict = GetInitJsonFile(vidPath, performersListStr, productionStudio);
      succeedCnt += MovieJsonDumper(dict, jsonPath);
      ++tryConstuctCnt;
    }
    if (tryConstuctCnt != succeedCnt) {
      qDebug("%d/%d json contructed succeed", succeedCnt, tryConstuctCnt);
    }
    return succeedCnt;
  }

  static int JsonPerformersKeyValuePairAdd(const QString& path) {
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
      if (dict.contains(JSONKey::Performers) and not dict[JSONKey::Performers].toStringList().isEmpty()){
        continue;
      }

      const QString& sentence = dict.contains(JSONKey::Name)? dict[JSONKey::Name].toString():"";
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

  static int JsonProductionStudiosKeyValuePairAdd(const QString& path) {
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
      if (dict.contains(JSONKey::ProductionStudio) and not dict[JSONKey::ProductionStudio].toString().isEmpty()){
        continue;
      }
      const QString& sentence = dict.contains(JSONKey::Name)? dict[JSONKey::Name].toString():"";
      dict.insert(JSONKey::ProductionStudio, psm(sentence));
      succeedCnt += MovieJsonDumper(dict, jsonPath);
      ++tryKVPairCnt;
    }
    if (tryKVPairCnt != succeedCnt) {
      qDebug("%d/%d json add production studio key-value pair succeed", succeedCnt, tryKVPairCnt);
    }
    return succeedCnt;
  }

  static int JsonValuePerformersAdder(const QString& path, const QString& performers) {
    const QStringList& performerList = performers.trimmed().split(SEPERATOR_COMP);
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

  static int JsonValueProductionStudioSetter(const QString& path, const QString& _productionStudio) {
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

  static const QMap<QString, QString> key2ValueType;
};

#endif  // JSONFILEHELPER_H
