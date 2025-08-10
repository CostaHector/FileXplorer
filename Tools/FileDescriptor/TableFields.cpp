#include "TableFields.h"
#include <QDebug>
#include "PublicMacro.h"

const char* DEV_DRV_TABLE::GetFILEDString(FIELD_E enum_val) {
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
      qWarning("enum_val[%d] out of range", enum_val);
      static const char UNKNOWN[]{"Unknown"};
      return UNKNOWN;
    }
  }
}
