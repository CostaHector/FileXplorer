#ifndef DATAFORMATTER_H
#define DATAFORMATTER_H

#include <QVariant>
#include <QString>
#include <QVector>

enum class Gender : int {
  male = 0,
  female,
};

class SortedUniqueStrContainer;

namespace DataFormatter {

QVariant formatDefault(const QVariant& v);
bool writeQString(QString& dst, const QVariant& src);
bool writeInt(int& dst, const QVariant& src);
bool writeFloat(float& dst, const QVariant& src);
bool writeDouble(double& dst, const QVariant& src);

const QString& formatSortedLst(const SortedUniqueStrContainer& container);
bool writeSortedLst(SortedUniqueStrContainer& container, const QVariant& src);

QString formatGender(Gender gen);
bool writeGender(Gender& dst, const QVariant& src);

QString formatPhoneNumber(const QString& phone);
bool writePhoneNumber(QString& dst, const QVariant& src);

QString formatFloat(float value);

QString formatRateAnnual(const QList<char>& rates);
bool writeRateAnnual(QList<char>& dst, const QVariant& src);

QString formatHobbies(QStringList hobbies);
bool writeHobbies(QStringList& dst, const QVariant& src);

QString formatBool(bool value);
bool writeBool(bool& dst, const QVariant& src);
};  // namespace DataFormatter

#endif  // DATAFORMATTER_H
