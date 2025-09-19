#include "DataFormatter.h"
#include "SortedUniqStrLst.h"
#include "NameTool.h"
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

namespace DataFormatter {

const QVariant& formatDefault(const QVariant& v) {  //
  return v;
}

QString formatFloat2Prec(float value) {  //
  return QString::number(value, 'f', 2);
}

bool writeQString(QString& dst, const QVariant& src) {
  QString srcValue{src.toString()};
  if (dst == srcValue) {
    return false;
  }
  dst.swap(srcValue);
  return true;
}

bool writeInt(int& dst, const QVariant& src) {
  int srcValue{src.toInt()};
  if (dst == srcValue) {
    return false;
  }
  std::swap(dst, srcValue);
  return true;
}

bool writeFloat(float& dst, const QVariant& src) {
  float srcValue{src.toFloat()};
  if (dst == srcValue) {
    return false;
  }
  std::swap(dst, srcValue);
  return true;
}
bool writeDouble(double& dst, const QVariant& src) {
  double srcValue{src.toDouble()};
  if (dst == srcValue) {
    return false;
  }
  std::swap(dst, srcValue);
  return true;
}

QString initQString(const QJsonObject& json, const QString& key, const QString& defaultValue) {
  return json.value(key).toString(defaultValue);
}

int initInt(const QJsonObject& json, const QString& key, const int& defaultValue) {
  return json.value(key).toInt(defaultValue);
}

QStringList initQStringLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue) {
  auto it = json.constFind(key);
  return it == json.constEnd() ? defaultValue : it->toVariant().toStringList();
}

SortedUniqStrLst initSortedLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue) {
  auto it = json.constFind(key);
  QStringList values{it == json.constEnd() ? defaultValue : it->toVariant().toStringList()};
  SortedUniqStrLst ans{values};
  return ans;
}

SortedUniqStrLst initCastSortedLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue) {
  // 1. standardlize Performer=>Cast
  auto it = json.constFind(key);
  if (it != json.constEnd()) {
    return SortedUniqStrLst{it->toVariant().toStringList()};
  }
  const auto performerIter = json.constFind("Performers");
  if (performerIter != json.constEnd()) {
    return SortedUniqStrLst{performerIter->toVariant().toStringList()};
  }
  return SortedUniqStrLst{defaultValue};
}

QString initStudioQString(const QJsonObject& json, const QString& key, const QString& defaultValue) {
  // 2. standardlize ProductionStudio=>Studio
  auto it = json.constFind(key);
  if (it != json.constEnd()) {
    return it->toString();
  } else {
    const auto performerIter = json.constFind("ProductionStudio");
    if (performerIter != json.constEnd()) {
      return performerIter->toString();
    }
  }
  return defaultValue;
}

const QString& formatSortedLst(const SortedUniqStrLst& container) {
  return container.join();
}

bool writeSortedLst(SortedUniqStrLst& container, const QVariant& src) {
  QString newTags = src.toString();
  if (container.join() == newTags) {
    return false;
  }
  container.setBatchFromSentence(newTags);
  return true;
}

QString formatQStringLst(const QStringList& container) {
  return container.join(NameTool::CSV_COMMA);
}

bool writeQStringLst(QStringList& container, const QVariant& src) {
  QStringList newTags = src.toString().split(NameTool::CSV_COMMA);
  if (container == newTags) {
    return false;
  }
  container.swap(newTags);
  return true;
}

QString formatGender(Gender gen) {  //
  return (gen == Gender::male) ? "male" : "female";
}

bool writeGender(Gender& dst, const QVariant& src) {
  QString srcStr{src.toString()};
  Gender srcValue{srcStr == "male" ? Gender::male : Gender::female};
  if (dst == srcValue) {
    return false;
  }
  std::swap(dst, srcValue);
  return true;
}

QString formatPhoneNumber(const QString& phone) {
  if (phone.length() < 11) {
    return phone;
  }
  return QString("%1-%2-%3-%4").arg(phone.left(2)).arg(phone.mid(2, 3)).arg(phone.mid(5, 4)).arg(phone.mid(9, 4));
}
bool writePhoneNumber(QString& dst, const QVariant& src) {
  QString srcStr{src.toString()};
  srcStr.remove('-');
  if (dst == srcStr) {
    return false;
  }
  dst.swap(srcStr);
  return true;
}

QString formatRateAnnual(const QList<char>& rates) {
  if (rates.isEmpty()) {
    return {};
  }
  QString s;
  s.reserve(rates.size() * 2);
  for (char c : rates) {
    s += c;
    s += ',';
  }
  s.chop(1);
  return s;
}

bool writeRateAnnual(QList<char>& dst, const QVariant& src) {
  QStringList lstStrValue = src.toString().split(',', Qt::SplitBehaviorFlags::SkipEmptyParts);
  QList<char> lstValue;
  lstValue.reserve(lstStrValue.size());
  for (QString& s : lstStrValue) {
    s = s.trimmed();
    if (s.isEmpty()) {
      continue;
    }
    lstValue.append(s[0].toLatin1());
  }
  if (dst == lstValue) {
    return false;
  }
  dst.swap(lstValue);
  return true;
}

QString formatHobbies(QStringList hobbies) {
  hobbies.sort(Qt::CaseSensitivity::CaseInsensitive);
  return hobbies.join(",");
}

bool writeHobbies(QStringList& dst, const QVariant& src) {
  QStringList lstValue = src.toString().split(',');
  if (dst == lstValue) {
    return false;
  }
  dst.swap(lstValue);
  return true;
}

QString formatBool(bool value) {  //
  return value ? "true" : "false";
}

bool writeBool(bool& dst, const QVariant& src) {  //
  QString srcStr{src.toString().toLower()};
  bool srcValue{false};
  if (srcStr == "true") {
    srcValue = true;
  } else if (srcStr == "false") {
    srcValue = false;
  }
  if (dst == srcValue) {
    return false;
  }
  std::swap(dst, srcValue);
  return true;
}

void writeJsonObjectInt(QJsonObject& json, const QString& key, const int& val) {
  json[key] = val;
}
void writeJsonObjectFloat(QJsonObject& json, const QString& key, const float& val) {
  json[key] = val;
}
void writeJsonObjectDouble(QJsonObject& json, const QString& key, const double& val) {
  json[key] = val;
}
void writeJsonObjectBool(QJsonObject& json, const QString& key, const bool& val) {
  json[key] = val;
}
void writeJsonObjectQString(QJsonObject& json, const QString& key, const QString& val) {
  json[key] = val;
}
void writeJsonObjectQStringLst(QJsonObject& json, const QString& key, const QStringList& val) {
  json[key] = QJsonArray::fromStringList(val);
}

void writeJsonObjectQIntList(QJsonObject& json, const QString& key, const QList<int>& val) {
  QJsonArray ja;
  for (auto ele : val) {
    ja.append(ele);
  }
  json[key] = ja;
}

void writeJsonObjectDefault(QJsonObject& json, const QString& key, const QVariant& val) {  //
  json[key] = QJsonValue::fromVariant(val);
}

void writeJsonObjectSortedStrLst(QJsonObject& json, const QString& key, const SortedUniqStrLst& val) {
  json[key] = QJsonArray::fromStringList(val.toSortedList());
}

};  // namespace DataFormatter
