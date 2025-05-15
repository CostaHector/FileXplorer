#ifndef JSONKEY_H
#define JSONKEY_H
#include "public/PublicMacro.h"
#include <QString>
#include <QVariant>
#include "DataFormatter.h"
//
//

namespace JsonKey {
/* Performers deprecated, Use Cast instead */
#define JSON_MODEL_FIELD_MAPPING                                                                                                            \
  JSON_KEY_ITEM(Prepath, 0, QString{}, QString{}, /*                          */ DataFormatter::formatDefault, DataFormatter::writeQString) \
  JSON_FILE_KEY_MAPPING

#define JSON_FILE_KEY_MAPPING                                                                                                                   \
  JSON_KEY_ITEM(Name, 1, QString{}, QString{}, /*                             */ DataFormatter::formatDefault, DataFormatter::writeQString)     \
  JSON_KEY_ITEM(Cast, 2, QStringList{}, SortedUniqueStrContainer{}, /*        */ DataFormatter::formatSortedLst, DataFormatter::writeSortedLst) \
  JSON_KEY_ITEM(Studio, 3, QString{}, QString{}, /*                           */ DataFormatter::formatDefault, DataFormatter::writeQString)     \
  JSON_KEY_ITEM(Tags, 4, QStringList{}, SortedUniqueStrContainer{}, /*        */ DataFormatter::formatSortedLst, DataFormatter::writeSortedLst) \
  JSON_KEY_ITEM(Detail, 5, QString{}, QString{}, /*                           */ DataFormatter::formatDefault, DataFormatter::writeQString)     \
  JSON_KEY_ITEM(Uploaded, 6, QString{}, QString{}, /*                         */ DataFormatter::formatDefault, DataFormatter::writeQString)     \
  JSON_KEY_ITEM(Rate, 7, (int)0, (int)0, /*                                   */ DataFormatter::formatDefault, DataFormatter::writeInt)         \
  JSON_KEY_ITEM(Size, 8, (int)0, (int)0, /*                                   */ DataFormatter::formatDefault, DataFormatter::writeInt)         \
  JSON_KEY_ITEM(Resolution, 9, QString{}, QString{}, /*                       */ DataFormatter::formatDefault, DataFormatter::writeQString)     \
  JSON_KEY_ITEM(Bitrate, 10, QString{}, QString{}, /*                         */ DataFormatter::formatDefault, DataFormatter::writeQString)     \
  JSON_KEY_ITEM(Hot, 11, QStringList{}, SortedUniqueStrContainer{}, /*        */ DataFormatter::formatSortedLst, DataFormatter::writeSortedLst) \
  JSON_KEY_ITEM(Duration, 12, (int)0, (int)0, /*                              */ DataFormatter::formatDefault, DataFormatter::writeInt)

// Key, enum like Name=0, Cast=1, ...,
enum JSON_KEY_E {
  JSON_KEY_BEGIN,
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer) enu = val,
  JSON_MODEL_FIELD_MAPPING
#undef JSON_KEY_ITEM
      JSON_KEY_BUTT,
};

// Default Value: variable like JSON_DEF_VAL_Name="", JSON_DEF_VAL_Cast=QStringList{}
#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer) static const auto JSON_DEF_VAL_##enu = def;
JSON_FILE_KEY_MAPPING
#undef JSON_KEY_ITEM

#define JSON_KEY_ITEM(enu, val, def, enhanceDef, format, writer) ENUM_TO_STRING(enu),
const QString JSON_KEY_IND_2_NAME[JSON_KEY_E::JSON_KEY_BUTT]{JSON_MODEL_FIELD_MAPPING};
#undef JSON_KEY_ITEM

QVariantHash GetJsonDictDefault(const QString& jsonBaseName = "");
}  // namespace JsonKey

enum class FIELD_OP_TYPE { CAST = 0, TAGS = 1, BUTT };
static const QString FIELF_OP_TYPE_ARR[(int)FIELD_OP_TYPE::BUTT]{
    ENUM_TO_STRING(CAST),
    ENUM_TO_STRING(TAGS),
};
enum class FIELD_OP_MODE { SET = 0, APPEND = 1, REMOVE = 2, BUTT };
static const QString FIELD_OP_MODE_ARR[(int)FIELD_OP_MODE::BUTT]{
    ENUM_TO_STRING(SET),
    ENUM_TO_STRING(APPEND),
    ENUM_TO_STRING(REMOVE),
};

#endif  // JSONKEY_H
