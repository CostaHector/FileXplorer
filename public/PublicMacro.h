#ifndef PUBLICMACRO_H
#define PUBLICMACRO_H
#include "Logger.h"

#define CHECK_NULLPTR_RETURN_VOID(pointer) \
  if (pointer == nullptr) {                \
    LOG_C("%s", #pointer " is nullptr");   \
    return;                                \
  }

#define CHECK_NULLPTR_RETURN_NULLPTR(pointer) \
  if (pointer == nullptr) {                   \
    LOG_C("%s", #pointer " is nullptr");      \
    return nullptr;                           \
  }

#define CHECK_NULLPTR_RETURN_FALSE(pointer) \
  if (pointer == nullptr) {                 \
    LOG_C("%s", #pointer " is nullptr");    \
    return false;                           \
  }

#define CHECK_NULLPTR_RETURN_INT(pointer, intVal) \
  if (pointer == nullptr) {                       \
    LOG_C("%s", #pointer " is nullptr");          \
    return intVal;                                \
  }

#define CHECK_NULLPTR_RETURN_DEFAULT_CONSTRUCT(pointer) \
  if (pointer == nullptr) {                             \
    LOG_C("%s", #pointer " is nullptr");                \
    return {};                                          \
  }

#define CHECK_FALSE_RETURN_VAL(bCondition, val) \
  if (!bCondition) {                            \
    LOG_E("Condition failed");                  \
    return val;                                 \
  }

#define CHECK_FALSE_RETURN_VOID(bCondition) \
  if (!bCondition) {                        \
    LOG_E("Condition failed");              \
    return;                                 \
  }

#define ENUM_2_STR(enum_val) #enum_val
#define CLASSNAME_2_STR(className) #className

#define CASE_BRANCH_ENUM_TO_STRING(enum_val) \
  case enum_val:                             \
    return #enum_val

#define STRINGIFY__LINE__CORE(lineNoLiteral) #lineNoLiteral
#define STRINGIFY__LINE__(lineNoMarco) STRINGIFY__LINE__CORE(lineNoMarco)
#define CSTRING_WITH__LINE__(specifiedCString, lineNoMarco) specifiedCString STRINGIFY__LINE__(lineNoMarco)

inline bool isIndexInrange(int begin, int val, int end) {
  return begin <= val && val < end;
}

#endif  // PUBLICMACRO_H
