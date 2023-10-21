#include "NameStandardizer.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include "PublicVariable.h"

auto NameStandardizer::jsonLoader() -> QMap<QString, QString> {
  QFile countries_file(":/STANDARD_STUDIO_NAME_JSON");
  QString json_string;
  QMap<QString, QString> name2standardName;
  if (countries_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    json_string = countries_file.readAll();
    countries_file.close();
  } else {
    qDebug("file not found");
    return name2standardName;
  }
  QJsonParseError jsonErr;
  QJsonDocument json_doc = QJsonDocument::fromJson(json_string.toUtf8(), &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError) {
    qDebug("Error when parse");
    return name2standardName;
  }
  QJsonObject rootObj = json_doc.object();
  for (const QString& k : rootObj.keys()) {
    const auto& v = rootObj.value(k);
    if (not v.isString()) {
      qDebug("Check value of key[%s], it may not be a string", k.toStdString().c_str());
      continue;
    }
    name2standardName[k] = rootObj.value(k).toString();
  }
  return name2standardName;
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  const auto& mp = NameStandardizer::jsonLoader();
  qDebug("STANDARD_STUDIO_NAME_JSON size=%d", mp.size());
}
#endif
