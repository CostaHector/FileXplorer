#include "TableFields.h"
#include <QDebug>
#include "PublicMacro.h"
namespace DEV_DRV_TABLE {
const char* GetFILEDString(FIELD_E enum_val) {
  switch (enum_val) {
    CASE_BRANCH_ENUM_TO_STRING(ROOT_PATH);
    CASE_BRANCH_ENUM_TO_STRING(VOLUME_LABEL);
    CASE_BRANCH_ENUM_TO_STRING(TOTAL_BYTES);
    CASE_BRANCH_ENUM_TO_STRING(AVAIL_BYTES);
    CASE_BRANCH_ENUM_TO_STRING(GUID);
    CASE_BRANCH_ENUM_TO_STRING(MOUNT_POINT);
    CASE_BRANCH_ENUM_TO_STRING(ADT_TIME);
    CASE_BRANCH_ENUM_TO_STRING(FILED_BUTT);
    default: {
      LOG_W("enum_val[%d] out of range", enum_val);
      static const char UNKNOWN[]{"Unknown"};
      return UNKNOWN;
    }
  }
}
}

namespace PERFORMER_DB_HEADER_KEY{
const char* GetFILEDString(FIELD_E enum_val) {
  switch (enum_val) {
#define PERFORMER_TABLE_KEY_ITEM(enu, val)   case enu: return #enu;
    PERFORMER_TABLE_FIELD_MAPPING
#undef PERFORMER_TABLE_KEY_ITEM
        default: {
      LOG_W("enum_val[%d] out of range", enum_val);
      static const char UNKNOWN[]{"Unknown"};
      return UNKNOWN;
    }
  }
}

const QStringList DB_HEADER {
#define PERFORMER_TABLE_KEY_ITEM(enu, val) #enu,
    PERFORMER_TABLE_FIELD_MAPPING
#undef PERFORMER_TABLE_KEY_ITEM
};
}
