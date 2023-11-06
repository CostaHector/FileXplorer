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

  static auto MovieJsonDumper(const QVariantMap& dict, const QString& movieJsonItemPath) -> bool {
    auto jsonObject = QJsonObject::fromVariantMap(dict);
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

  static auto MovieJsonLoader(const QString& movieJsonItemPath) -> QHash<QString, QJsonValue> {
    QFile jsonFile(movieJsonItemPath);
    QString json_string;
    QHash<QString, QJsonValue> movieJson;
    if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      json_string = jsonFile.readAll();
      jsonFile.close();
    } else {
      qDebug("file not found");
      return movieJson;
    }
    QJsonParseError jsonErr;
    QJsonDocument json_doc = QJsonDocument::fromJson(json_string.toUtf8(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError) {
      qDebug("Error when parse");
      return movieJson;
    }
    QJsonObject rootObj = json_doc.object();
    for (const QString& k : rootObj.keys()) {
      const auto& v = rootObj.value(k);
      movieJson.insert(k, rootObj.value(k));
    }
    return movieJson;
  }

  static auto MapToOrderedList(const QHash<QString, QJsonValue>& in) -> QList<QPair<QString, QJsonValue>> {
    QList<QPair<QString, QJsonValue>> out;
    for (auto it = in.cbegin(); it != in.cend(); ++it) {
      out.append(qMakePair(it.key(), it.value()));
    }
    std::sort(out.begin(), out.end(), JSONKey::KeySorter);
    return out;
  }

  static auto PerformersString2JsonArry(const QString& valueStr) -> QJsonArray {
    if (valueStr.isEmpty()) {
      return {};
    }
    QJsonArray arr;
    for (const QString& perfRaw : valueStr.split(SEPERATOR_COMP)) {
      const QJsonValue& perf = perfRaw.trimmed();
      if (not arr.contains(perf)) {
        arr << perf;
      }
    }
    return arr;
  }
};

#endif  // JSONFILEHELPER_H
