#ifndef DATAFORMATTER_H
#define DATAFORMATTER_H

#include <QVariant>
#include <QString>
#include <QVector>

enum class Gender : int {
  male = 0,
  female,
};

class SortedUniqStrLst;

namespace DataFormatter {

// used in setData(), input QVariant, output: specified data memeber type
bool writeQString(QString& dst, const QVariant& src);
bool writeInt(int& dst, const QVariant& src);
bool writeqint64(qint64& dst, const QVariant& src);
bool writeFloat(float& dst, const QVariant& src);
bool writeDouble(double& dst, const QVariant& src);
bool writeQByteArray(QByteArray& dst, const QVariant& src);
bool writeSortedLst(SortedUniqStrLst& container, const QVariant& src);
bool writeQStringLst(QStringList& container, const QVariant& src);
bool writeGender(Gender& dst, const QVariant& src);
bool writePhoneNumber(QString& dst, const QVariant& src);
bool writeRateAnnual(QList<char>& dst, const QVariant& src);
bool writeHobbies(QStringList& dst, const QVariant& src);
bool writeBool(bool& dst, const QVariant& src);

// used in data member value initialization/reload when read out from json
QString initQString(const QJsonObject& json, const QString& key, const QString& defaultValue);
QString initStudioQString(const QJsonObject& json, const QString& key, const QString& defaultValue);
int initInt(const QJsonObject& json, const QString& key, const int& defaultValue);
qint64 initFileSizeQint64(const QJsonObject& json, const QString& key, const qint64& defaultValue);
bool initBool(const QJsonObject& json, const QString& key, const bool& defaultValue);
QStringList initQStringLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue);
SortedUniqStrLst initSortedLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue);
SortedUniqStrLst initCastSortedLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue);
QByteArray initQByteArray(const QJsonObject& json, const QString& key, const QByteArray& defaultValue);

// used in construct json from data member value.
void writeJsonObjectInt(QJsonObject& json, const QString& key, const int& val);
void writeJsonObjectFileSizeQint64(QJsonObject& json, const QString& key, const qint64& val);
void writeJsonObjectFloat(QJsonObject& json, const QString& key, const float& val);
void writeJsonObjectDouble(QJsonObject& json, const QString& key, const double& val);
void writeJsonObjectBool(QJsonObject& json, const QString& key, const bool& val);
void writeJsonObjectQString(QJsonObject& json, const QString& key, const QString& val);
void writeJsonObjectQStringLst(QJsonObject& json, const QString& key, const QStringList& val);
void writeJsonObjectQIntList(QJsonObject& json, const QString& key, const QList<int>& val);
void writeJsonObjectDefault(QJsonObject& json, const QString& key, const QVariant& val);
void writeJsonObjectSortedStrLst(QJsonObject& json, const QString& key, const SortedUniqStrLst& val);
void writeJsonObjectQByteArray(QJsonObject& json, const QString& key, const QByteArray& val);

// used in data(), data member type to human readable items
const QVariant& formatDefault(const QVariant& v);
QString formatFloat2Prec(float value);
QString formatBool(bool value);
const QString& formatSortedLst(const SortedUniqStrLst& container);
QString formatQStringLst(const QStringList& container);
QString formatPhoneNumber(const QString& phone);
QString formatGender(Gender gen);
QString formatRateAnnual(const QList<char>& rates);
QString formatHobbies(QStringList hobbies);

// const qint64 xGiB = total / (1 << 30);
// const qint64 xMiB = total % (1 << 30) / (1 << 20);
// const qint64 xkiB = total % (1 << 30) % (1 << 20) / (1 << 10);
// const qint64 xB = total % (1 << 30) % (1 << 20) % (1 << 10);
inline QString formatFileSizeGMKB(const qint64 total) {
  static constexpr qint64 GiB = 1 << 30;
  static constexpr qint64 MiB = 1 << 20;
  static constexpr qint64 KiB = 1 << 10;

  const qint64 xGiB = total >> 30;
  const qint64 remGiB = total & (GiB - 1);

  const qint64 xMiB = remGiB >> 20;
  const qint64 remMiB = remGiB & (MiB - 1);

  const qint64 xkiB = remMiB >> 10;
  const qint64 xB = remMiB & (KiB - 1);

  static const QString FILE_SIZE_GMKB_TEMPLATE{"%1'%2'%3'%4"};
  return FILE_SIZE_GMKB_TEMPLATE.arg(xGiB).arg(xMiB).arg(xkiB).arg(xB);
}
inline QString formatFileSizeWithBytes(const qint64 total) {
  const QString FILE_SIZE_DETAIL_TEMPLATE{"%1 (%2 Bytes)"};
  return FILE_SIZE_DETAIL_TEMPLATE.arg(formatFileSizeGMKB(total)).arg(total);
}

QString formatDateIsoMs(const qint64 ms);
QString formatDateIso(const qint64 ms);

QString formatDurationISOMs(const qint64 ms);
QString formatDurationISO(const qint64 ms);
}  // namespace DataFormatter

#endif  // DATAFORMATTER_H
