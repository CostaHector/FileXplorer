#ifndef DBMANAGERHELPER_H
#define DBMANAGERHELPER_H

#include "PublicMacro.h"
#include <QString>

#define SQL_TABLE_DROP_OR_DELETE_MAPPING \
  SQL_TABLE_DROP_OR_DELETE_ITEM(DROP, 0, "DROP TABLE `%1`;") \
  SQL_TABLE_DROP_OR_DELETE_ITEM(DELETE, 1, "DELETE FROM `%1`;")

namespace DbManagerHelper {
enum DropOrDeleteE {
#define SQL_TABLE_DROP_OR_DELETE_ITEM(enu, val, cmds) enu = val,
  SQL_TABLE_DROP_OR_DELETE_MAPPING
#undef SQL_TABLE_DROP_OR_DELETE_ITEM
};

inline const char* c_str(DropOrDeleteE dropOrDelete) {
  static constexpr const char* DROP_OR_DELETE_E_2_CHAR_ARRAY[]{
#define SQL_TABLE_DROP_OR_DELETE_ITEM(enu, val, cmds) ENUM_2_STR(enu),
      SQL_TABLE_DROP_OR_DELETE_MAPPING
#undef SQL_TABLE_DROP_OR_DELETE_ITEM
  };
  static constexpr int DI_TABLE_HEADERS_COUNT = sizeof(DROP_OR_DELETE_E_2_CHAR_ARRAY) / sizeof(DROP_OR_DELETE_E_2_CHAR_ARRAY[0]);

  if ((int) dropOrDelete < 0 || (int) dropOrDelete >= DI_TABLE_HEADERS_COUNT) {
    return "unknown dropOrDelete";
  }

  return DROP_OR_DELETE_E_2_CHAR_ARRAY[(int) dropOrDelete];
}

inline QString command(DropOrDeleteE dropOrDelete) {
  static constexpr const char* DROP_OR_DELETE_E_2_COMMANDS_ARRAY[]{
#define SQL_TABLE_DROP_OR_DELETE_ITEM(enu, val, cmds) cmds,
      SQL_TABLE_DROP_OR_DELETE_MAPPING
#undef SQL_TABLE_DROP_OR_DELETE_ITEM
  };
  static constexpr int DI_TABLE_HEADERS_COUNT = sizeof(DROP_OR_DELETE_E_2_COMMANDS_ARRAY) / sizeof(DROP_OR_DELETE_E_2_COMMANDS_ARRAY[0]);

  if ((int) dropOrDelete < 0 || (int) dropOrDelete >= DI_TABLE_HEADERS_COUNT) {
    return "unknown dropOrDelete";
  }

  return DROP_OR_DELETE_E_2_COMMANDS_ARRAY[(int) dropOrDelete];
}

} // namespace DbManagerHelper

#endif // DBMANAGERHELPER_H
