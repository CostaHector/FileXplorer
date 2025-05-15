#include "JsonKey.h"
#include <QVariantHash>
#include <QHash>

namespace JsonKey {
QVariantHash GetJsonDictDefault(const QString& jsonBaseName) {  //  QVariantHash dict {
  QVariantHash dict{
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer) {ENUM_TO_STRING(enu), def},
      JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM
  };
  dict[ENUM_TO_STRING(Name)] = jsonBaseName;
  return dict;
}
}  // namespace JsonKey
