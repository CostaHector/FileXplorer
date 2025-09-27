#include "JsonKey.h"
#include "JsonHelper.h"
#include <QVariantHash>
#include <QHash>

namespace JsonKey {
QVariantHash GetJsonDictDefault(const QString& jsonBaseName) {  //  QVariantHash dict {
  QVariantHash dict{
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer, initer, jsonWriter) {ENUM_2_STR(enu), def},
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
  };
  dict[ENUM_2_STR(Name)] = jsonBaseName;
  return dict;
}

QVariantHash ConstructJsonDict(
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer, initer, jsonWriter) decltype(def) _##enu,
    JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
    void* P_DONT_USE_ME) {
  QVariantHash dict{
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer, initer, jsonWriter) {ENUM_2_STR(enu), _##enu},
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
  };
  return dict;
}

QByteArray ConstructJsonByteArray(
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer, initer, jsonWriter) decltype(def) _##enu,
    JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
    void* P_DONT_USE_ME) {

  QVariantHash dict = ConstructJsonDict(
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer, initer, jsonWriter) _##enu,
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
      P_DONT_USE_ME);

  return JsonHelper::SerializedJsonDict2ByteArray(dict);
}

}  // namespace JsonKey
