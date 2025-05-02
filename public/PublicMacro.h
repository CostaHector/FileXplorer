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

#define CHECK_NULLPTR_RETURN_FALSE(pointer)   \
  if (pointer == nullptr) {                   \
      qCritical("%s", #pointer " is nullptr");\
      return false;                           \
  }

#define VOLUME_ENUM_TO_STRING(enum_val) #enum_val

#endif  // PUBLICMACRO_H
