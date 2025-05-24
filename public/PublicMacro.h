#ifndef PUBLICMACRO_H
#define PUBLICMACRO_H

#define CHECK_NULLPTR_RETURN_VOID(pointer)   \
  if (pointer == nullptr) {                  \
    qCritical("%s", #pointer " is nullptr"); \
    return;                                  \
  }

#define CHECK_NULLPTR_RETURN_NULLPTR(pointer) \
  if (pointer == nullptr) {                   \
    qCritical("%s", #pointer " is nullptr");  \
    return nullptr;                           \
  }

#define CHECK_NULLPTR_RETURN_FALSE(pointer)  \
  if (pointer == nullptr) {                  \
    qCritical("%s", #pointer " is nullptr"); \
    return false;                            \
  }

#define ENUM_2_STR(enum_val) #enum_val
#define CLASSNAME_2_STR(className) #className

#define CASE_BRANCH_ENUM_TO_STRING(enum_val) \
  case enum_val:                             \
    return #enum_val

#endif  // PUBLICMACRO_H
