#ifndef DEVICEDRIVERDBMODELFIELD_H
#define DEVICEDRIVERDBMODELFIELD_H

#include <QStringList>
#include "PublicMacro.h"

#define DEV_DRV_TABLE_ENUM_VALUE_MAPPING \
  DEV_DRV_TABLE_ENUM_ITEM(ROOT_PATH, 0) \
  DEV_DRV_TABLE_ENUM_ITEM(TOTAL_BYTES, 1) \
  DEV_DRV_TABLE_ENUM_ITEM(USED_BYTES, 2) \

namespace DeviceDriverDBModelField {
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
  if (devDrvFieldE < DeviceDriverDBModelField::BEGIN || devDrvFieldE >= DeviceDriverDBModelField::FILED_BUTT) {
    return "unknown dv criteria";
  }
  static constexpr const char* DEV_DRV_TABLE_E_2_CHAR_ARRAY[(int) DeviceDriverDBModelField::FILED_BUTT]{
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

#undef DEV_DRV_TABLE_ENUM_VALUE_MAPPING

#endif // DEVICEDRIVERDBMODELFIELD_H
