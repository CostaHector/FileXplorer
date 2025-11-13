#ifndef TABLEFIELDS_H
#define TABLEFIELDS_H

#include <QStringList>
#include "PublicMacro.h"

#define DEV_DRV_TABLE_ENUM_VALUE_MAPPING \
  DEV_DRV_TABLE_ENUM_ITEM(ROOT_PATH, 0) \
  DEV_DRV_TABLE_ENUM_ITEM(TOTAL_BYTES, 1) \
  DEV_DRV_TABLE_ENUM_ITEM(USED_BYTES, 2) \

namespace DEV_DRV_TABLE {
enum FIELD_E {
  // add after BEGIN
  BEGIN = 0,
#define DEV_DRV_TABLE_ENUM_ITEM(enu, val) enu = val,
  DEV_DRV_TABLE_ENUM_VALUE_MAPPING
#undef DEV_DRV_TABLE_ENUM_ITEM
      // add before BUTT
      FILED_BUTT
};

inline const char* c_str(FIELD_E devDrvFieldE) {
  if (devDrvFieldE < DEV_DRV_TABLE::BEGIN || devDrvFieldE >= DEV_DRV_TABLE::FILED_BUTT) {
    return "unknown dv criteria";
  }
  static constexpr const char* DEV_DRV_TABLE_E_2_CHAR_ARRAY[(int) DEV_DRV_TABLE::FILED_BUTT]{
#define DEV_DRV_TABLE_ENUM_ITEM(enu, val) ENUM_2_STR(enu),
      DEV_DRV_TABLE_ENUM_VALUE_MAPPING
#undef DEV_DRV_TABLE_ENUM_ITEM
  };
  return DEV_DRV_TABLE_E_2_CHAR_ARRAY[(int) devDrvFieldE];
}

inline const QStringList& GetDevDrvTableHeaders() {
  static const QStringList DevDrvTableHeaders {
#define DEV_DRV_TABLE_ENUM_ITEM(enu, val) ENUM_2_STR(enu),
    DEV_DRV_TABLE_ENUM_VALUE_MAPPING
#undef DEV_DRV_TABLE_ENUM_ITEM
  };
  return DevDrvTableHeaders;
}

} // namespace DEV_DRV_TABLE

namespace MainKey {
enum FIELD_E {
  Name = 0,     //
  Size,         //
  Type,         //
  DateModified, //
};
}

namespace MOVIE_TABLE {
enum FIELD_E {
  Fd = 0,       //
  PrePathLeft,  //
  PrePathRight, //
  Name,         //
  Size,         //
  Duration,     //
  Studio,       //
  Cast,         //
  Tags,         //
  PathHash,     //
  BUTT
};
}

namespace TORRENTS_DB_HEADER_KEY {
enum FIELD_E {
  Name = 0,
  Size,
  DateModified,
  MD5,
  PrePath,
  BUTT,
};
} // namespace TORRENTS_DB_HEADER_KEY

#define PERFORMER_TABLE_FIELD_MAPPING \
  PERFORMER_TABLE_KEY_ITEM(Name, 0) \
  PERFORMER_TABLE_KEY_ITEM(Rate, 1) \
  PERFORMER_TABLE_KEY_ITEM(AKA, 2) \
  PERFORMER_TABLE_KEY_ITEM(Tags, 3) \
  PERFORMER_TABLE_KEY_ITEM(Ori, 4) \
  PERFORMER_TABLE_KEY_ITEM(Vids, 5) \
  PERFORMER_TABLE_KEY_ITEM(Imgs, 6) \
  PERFORMER_TABLE_KEY_ITEM(Detail, 7) \
  PERFORMER_TABLE_KEY_ITEM(Height, 8) \
  PERFORMER_TABLE_KEY_ITEM(Size, 9) \
  PERFORMER_TABLE_KEY_ITEM(Birth, 10) \

namespace PERFORMER_DB_HEADER_KEY {
enum FIELD_E {
#define PERFORMER_TABLE_KEY_ITEM(enu, val) enu = val,
  PERFORMER_TABLE_FIELD_MAPPING
#undef PERFORMER_TABLE_KEY_ITEM
};

constexpr const char* CAST_TABLE_HEADERS[]{
#define PERFORMER_TABLE_KEY_ITEM(enu, enumVal) #enu,
    PERFORMER_TABLE_FIELD_MAPPING
#undef PERFORMER_TABLE_KEY_ITEM
};
constexpr int CAST_TABLE_HEADERS_COUNT = sizeof(CAST_TABLE_HEADERS) / sizeof(CAST_TABLE_HEADERS[0]);

} // namespace PERFORMER_DB_HEADER_KEY

#endif // TABLEFIELDS_H
