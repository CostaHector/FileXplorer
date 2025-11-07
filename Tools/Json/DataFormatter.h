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

const QVariant& formatDefault(const QVariant& v);
QString formatFloat2Prec(float value);

bool writeQString(QString& dst, const QVariant& src);
bool writeInt(int& dst, const QVariant& src);
bool writeFloat(float& dst, const QVariant& src);
bool writeDouble(double& dst, const QVariant& src);
QString initQString(const QJsonObject& json, const QString& key, const QString& defaultValue);
QString initStudioQString(const QJsonObject& json, const QString& key, const QString& defaultValue);
int initInt(const QJsonObject& json, const QString& key, const int& defaultValue);
QStringList initQStringLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue);
SortedUniqStrLst initSortedLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue);
SortedUniqStrLst initCastSortedLst(const QJsonObject& json, const QString& key, const QStringList& defaultValue);
void writeJsonObjectInt(QJsonObject& json, const QString& key, const int& val);
void writeJsonObjectFloat(QJsonObject& json, const QString& key, const float& val);
void writeJsonObjectDouble(QJsonObject& json, const QString& key, const double& val);
void writeJsonObjectBool(QJsonObject& json, const QString& key, const bool& val);
void writeJsonObjectQString(QJsonObject& json, const QString& key, const QString& val);
void writeJsonObjectQStringLst(QJsonObject& json, const QString& key, const QStringList& val);
void writeJsonObjectQIntList(QJsonObject& json, const QString& key, const QList<int>& val);
void writeJsonObjectDefault(QJsonObject& json, const QString& key, const QVariant& val);
void writeJsonObjectSortedStrLst(QJsonObject& json, const QString& key, const SortedUniqStrLst& val);

const QString& formatSortedLst(const SortedUniqStrLst& container);
bool writeSortedLst(SortedUniqStrLst& container, const QVariant& src);

QString formatQStringLst(const QStringList& container);
bool writeQStringLst(QStringList& container, const QVariant& src);

QString formatGender(Gender gen);
bool writeGender(Gender& dst, const QVariant& src);

QString formatPhoneNumber(const QString& phone);
bool writePhoneNumber(QString& dst, const QVariant& src);

QString formatRateAnnual(const QList<char>& rates);
bool writeRateAnnual(QList<char>& dst, const QVariant& src);

QString formatHobbies(QStringList hobbies);
bool writeHobbies(QStringList& dst, const QVariant& src);

QString formatBool(bool value);
bool writeBool(bool& dst, const QVariant& src);


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

QString formatMd5ByPathFirst8Byte(const QString& fileAbspath);
QString formatMd5ByPathFirst1MillionByte(const QString& fileAbspath);
QString formatMd5ByPath(const QString& fileAbspath);
}  // namespace DataFormatter

#endif  // DATAFORMATTER_H
