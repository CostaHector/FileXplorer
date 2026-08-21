#include "CastPsonFileHelper.h"
#include "PublicMacro.h"
#include "JsonHelper.h"
#include <QSqlField>

namespace CastPsonFileHelper {

QString PsonPath(const QString& imageHostPath, const QVariantHash& pson) {
  using namespace CastDbModelField;
  return PsonPath(imageHostPath, pson[ENUM_2_STR(Ori)].toString(), pson[ENUM_2_STR(Name)].toString());
}

QVariantHash PerformerJsonJoiner(const QSqlRecord& record) {
  using namespace CastDbModelField;

  return PerformerJsonJoiner(
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) record.field(enu).value().sqlRecordToValueFunc(),
      PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
      nullptr);
}

QVariantHash PerformerJsonJoiner(
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) decltype(defaultValue) _##enu,
    PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
    void* P_DONT_USE_ME) {
  /* Ori can be
  Science Fiction/Sci-Fi
  Adventure Movie
  Drama/Feature Movie
  Documentary
  Superhero Movie
  Horror Movie
  Action Movie
  Comedy
  Thriller
  Fantasy
  */

  using namespace CastDbModelField;
  return {
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) {ENUM_2_STR(enu), _##enu}, //
      PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
  };
}

QByteArray CastValues2PsonStr(
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) decltype(defaultValue) _##enu,
    PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
    void* P_DONT_USE_ME) {

  QVariantHash varHash{PerformerJsonJoiner(
#define PSON_KEY_ITEM(enu, enumVal, defaultValue, sqlRecordToValueFunc, tblFieldDefinition) _##enu, //
      PSON_MODEL_FIELD_MAPPING
#undef PSON_KEY_ITEM
      nullptr)};

  return JsonHelper::SerializedJsonDict2ByteArray(varHash);
}

} // namespace CastPsonFileHelper
