#ifndef CASTPSONFILEHELPER_H
#define CASTPSONFILEHELPER_H

#include <QSqlRecord>
#include <QString>
#include <QVariantHash>

// enum/memeber/parameter name, enumValue, defaultValueAndType, sqlRecordToType, tblFieldDefinition
#define PSON_MODEL_FIELD_MAPPING \
  PSON_KEY_ITEM(Name, 0, QString{}, toString, QString{R"(`%1` TEXT NOT NULL %2,)"}) \
  PSON_KEY_ITEM(Rate, 1, (int) 0, toInt, QString{R"(`%1` INT DEFAULT %2,)"}) \
  PSON_KEY_ITEM(AKA, 2, QString{}, toString, QString{R"(`%1` TEXT DEFAULT "%2",)"}) \
  PSON_KEY_ITEM(Tags, 3, QString{}, toString, QString{R"(`%1` TEXT DEFAULT "%2",)"}) \
  PSON_KEY_ITEM(Ori, 4, QString{"Football"}, toString, QString{R"(`%1` TEXT DEFAULT "%2",)"}) \
  PSON_KEY_ITEM(Height, 5, (int) -1, toInt, QString{R"(`%1` INT DEFAULT %2,)"}) \
  PSON_KEY_ITEM(Size, 6, (int) -1, toInt, QString{R"(`%1` INT DEFAULT %2,)"}) \
  PSON_KEY_ITEM(Birth, 7, QString{}, toString, QString{R"(`%1` TEXT DEFAULT "%2",)"}) \
  PSON_KEY_ITEM(Vids, 8, QString{}, toString, QString{R"(`%1` TEXT DEFAULT "%2",)"}) \
  PSON_KEY_ITEM(Imgs, 9, QString{}, toString, QString{R"(`%1` TEXT DEFAULT "%2",)"}) \
  PSON_KEY_ITEM(Detail, 10, QString{}, toString, QString{R"(`%1` TEXT DEFAULT "%2",)"})

namespace PERFORMER_DB_HEADER_KEY {
enum FIELD_E {
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) enu = enumVal,
  PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
};

constexpr const char* CAST_TABLE_HEADERS[]{
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) #enu,
    PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
};
constexpr int CAST_TABLE_HEADERS_COUNT = sizeof(CAST_TABLE_HEADERS) / sizeof(CAST_TABLE_HEADERS[0]);

} // namespace PERFORMER_DB_HEADER_KEY

namespace CastPsonFileHelper {
constexpr int DEFAULT_RATE{0};

QString PsonPath(const QString& imageHostPath, const QVariantHash& pson);

inline QString PsonPath(const QString& imageHostPath, const QString& ori, const QString& castName) {
  return imageHostPath + '/' + ori + '/' + castName + '/' + castName + ".pson";
}

QVariantHash PerformerJsonJoiner(const QSqlRecord& record);

QVariantHash PerformerJsonJoiner(
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) decltype(defaultValue) _##enu = defaultValue,
    PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
    void* P_DONT_USE_ME
    = nullptr);

QByteArray CastValues2PsonStr(
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) decltype(defaultValue) _##enu = defaultValue,
    PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
    void* P_DONT_USE_ME
    = nullptr);
} // namespace CastPsonFileHelper

#endif // CASTPSONFILEHELPER_H
