#ifndef JSONFILEHELPER_H
#define JSONFILEHELPER_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include "PublicVariable.h"

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
    QTextStream in(&jsonFile);
    in.setCodec("UTF-8");
    in << byteArray;
    jsonFile.close();
    return true;
  }

  static auto MovieJsonLoader(const QString& movieJsonItemPath) -> QVariantHash {
    QFile jsonFile(movieJsonItemPath);
    QString json_string;
    if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      json_string = jsonFile.readAll();
      jsonFile.close();
    } else {
      qDebug("[Nomarl] json file not found");
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

  static const QMap<QString, QString> key2ValueType;
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
};

#endif  // JSONFILEHELPER_H
