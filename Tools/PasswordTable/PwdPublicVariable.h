#ifndef PWDPUBLICVARIABLE_H
#define PWDPUBLICVARIABLE_H

#include "PublicMacro.h"
#define PWD_TABLE_TYPES_ENUM_VALUE_MAPPING \
  PWD_TABLE_TYPES_ITEM(INDEX, 0)           \
  PWD_TABLE_TYPES_ITEM(TYPE, 1)            \
  PWD_TABLE_TYPES_ITEM(NAME, 2)            \
  PWD_TABLE_TYPES_ITEM(ACCOUNT, 3)

#include <QFont>

namespace PwdPublicVariable {
extern const QFont TEXT_EDIT_FONT;
enum class SAVE_RESULT { OK = 0, SKIP = 1, FAILED = 2, BUTT };
constexpr char SAVE_RESULT_STR[(int)SAVE_RESULT::BUTT][10]{"OK", "SKIP", "FAILED"};

enum PwdTypeE {
  BEGIN_DEFAULT = 0,
// add after BEGIN
#define PWD_TABLE_TYPES_ITEM(enu, val) enu = val,
  PWD_TABLE_TYPES_ENUM_VALUE_MAPPING
#undef PWD_TABLE_TYPES_ITEM
      // add before BUTT
      END_INVALID,
};

constexpr const char* PWD_TABLE_HEADERS[]{
#define PWD_TABLE_TYPES_ITEM(enu, enumVal) #enu,
    PWD_TABLE_TYPES_ENUM_VALUE_MAPPING
#undef PWD_TABLE_TYPES_ITEM
};
constexpr int PWD_TABLE_HEADERS_COUNT = sizeof(PWD_TABLE_HEADERS) / sizeof(PWD_TABLE_HEADERS[0]);

}  // namespace PwdPublicVariable

#endif  // PWDPUBLICVARIABLE_H
