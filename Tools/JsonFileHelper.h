#ifndef JSONFILEHELPER_H
#define JSONFILEHELPER_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

#include <QDirIterator>
#include <QFile>

namespace JSONKey {
const QString Name = "Name";
const QString Performers = "Performers";
const QString ProductionStudio = "ProductionStudio";
const QString Uploaded = "Uploaded";
const QString Tags = "Tags";
const QString Rate = "Rate";
const QString Size = "Size";
const QString Resolution = "Resolution";
const QString Bitrate = "Bitrate";
const QString Hot = "Hot";
const QString Detail = "Detail";
const QStringList JsonKeyListOrder{Name, Performers, ProductionStudio, Uploaded, Tags, Rate, Size, Resolution, Bitrate, Hot, Detail};
const QHash<QString, int> JsonKeyPri = {{Name, JsonKeyListOrder.indexOf(Name)},
                                        {Performers, JsonKeyListOrder.indexOf(Performers)},
                                        {ProductionStudio, JsonKeyListOrder.indexOf(ProductionStudio)},
                                        {Uploaded, JsonKeyListOrder.indexOf(Uploaded)},
                                        {Tags, JsonKeyListOrder.indexOf(Tags)},
                                        {Rate, JsonKeyListOrder.indexOf(Rate)},
                                        {Size, JsonKeyListOrder.indexOf(Size)},
                                        {Resolution, JsonKeyListOrder.indexOf(Resolution)},
                                        {Bitrate, JsonKeyListOrder.indexOf(Bitrate)},
                                        {Hot, JsonKeyListOrder.indexOf(Hot)},
                                        {Detail, JsonKeyListOrder.indexOf(Detail)}};
bool JsonKeySorter(const QPair<QString, QVariant>& l, const QPair<QString, QVariant>& r);
}  // namespace JSONKey

class JsonFileHelper {
 public:
  static bool MovieJsonDumper(const QVariantHash& dict, const QString& movieJsonItemPath);

  static QVariantHash JsonStr2Dict(const QString& jsonStr);
  static QVariantHash MovieJsonLoader(const QString& movieJsonItemPath);

  static auto MapToOrderedList(const QVariantHash& in) -> QList<QPair<QString, QVariant>> {
    QList<QPair<QString, QVariant>> out;
    for (auto it = in.cbegin(); it != in.cend(); ++it) {
      out.append(qMakePair(it.key(), it.value()));
    }
    std::sort(out.begin(), out.end(), JSONKey::JsonKeySorter);
    return out;
  }

  static auto HotSceneString2IntList(const QString& valueStr) -> QList<QVariant>;

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
        qDebug("type(map[%s]) cannot be processed", qPrintable(keyName));
      }
    } else {
      valueStr = v.toString();
    }
    return valueStr;
  }

  static QVariantHash GetMovieFileJsonDict(const QString& fileAbsPath, const QString& performersListStr = "", const QString& productionStudio = "");

  static QVariantHash GetDefaultJsonFile(const QString& fileName = "");

  static QString GetJsonFilePath(const QString& vidsPath) {
    const int sufLen = vidsPath.lastIndexOf('.');
    const QString& jsonPath = vidsPath.left(sufLen) + ".json";
    return jsonPath;
  }

  static int ConstructJsonForVids(const QString& path, const QString& productionStudio = "", const QString& performersListStr = "");

  static int JsonPerformersKeyValuePairAdd(const QString& path);

  static int JsonProductionStudiosKeyValuePairAdd(const QString& path);

  static int JsonValuePerformersProductionStudiosCleaner(const QString& path);

  static int JsonValuePerformersAdder(const QString& path, const QString& performers);

  static int JsonValueProductionStudioSetter(const QString& path, const QString& _productionStudio);

  static const QMap<QString, QString> key2ValueType;
};

#endif  // JSONFILEHELPER_H
