#include "TableFields.h"
#include <QDebug>

// 定义一个辅助宏来简化枚举到字符串的映射
#define CASE_ENUM_TO_STRING(enum_val) \
  case enum_val:                      \
    return #enum_val

const char* DEV_DRV_TABLE::GetFILEDString(FIELD_E enum_val) {
  switch (enum_val) {
    CASE_ENUM_TO_STRING(ROOT_PATH);
    CASE_ENUM_TO_STRING(VOLUME_LABEL);
    CASE_ENUM_TO_STRING(TOTAL_BYTES);
    CASE_ENUM_TO_STRING(AVAIL_BYTES);
    CASE_ENUM_TO_STRING(GUID);
    CASE_ENUM_TO_STRING(MOUNT_POINT);
    CASE_ENUM_TO_STRING(ADT_TIME);
    CASE_ENUM_TO_STRING(FILED_BUTT);
    default: {
      qWarning("enum_val[%d] out of range", enum_val);
      static const char UNKNOWN[]{"Unknown"};
      return UNKNOWN;
    }
  }
}
