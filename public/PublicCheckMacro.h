#ifndef PUBLICCHECKMACRO_H
#define PUBLICCHECKMACRO_H
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
  if (!(bCondition)) {                          \
    LOG_E("Condition failed");                  \
    return val;                                 \
  }

#define CHECK_FALSE_RETURN_VOID(bCondition) \
  if (!(bCondition)) {                      \
    LOG_E("Condition failed");              \
    return;                                 \
  }

#endif  // PUBLICCHECKMACRO_H
