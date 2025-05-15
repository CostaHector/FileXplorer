#include "DataFormatter.h"
#include "SortedUniqueStrContainer.h"

namespace DataFormatter {

QVariant formatDefault(const QVariant& v) {  //
  return v;
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

const QString& formatSortedLst(const SortedUniqueStrContainer& container) {
  return container.join();
}

bool writeSortedLst(SortedUniqueStrContainer& container, const QVariant& src) {
  QString newTags = src.toString();
  if (container.join() == newTags) {
    return false;
  }
  container.setBatchFromSentence(newTags);
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
  if (phone.length() < 11)
    return phone;
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
QString formatFloat(float value) {  //
  return QString::number(value, 'f', 2);
}

QString formatRateAnnual(const QList<char>& rates) {
  if (rates.isEmpty()) {
    return {};
  }
  QString s;
  s.reserve(rates.size() * 2);
  for (char c : rates) {
    s += ',';
    s += c;
  }
  return s;
}

bool writeRateAnnual(QList<char>& dst, const QVariant& src) {
  QStringList lstStrValue = src.toString().split(',');
  QList<char> lstValue;
  lstValue.reserve(lstStrValue.size());
  for (const auto& s : lstStrValue) {
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
  QString srcStr{src.toString()};
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
};  // namespace DataFormatter
