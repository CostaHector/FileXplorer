#ifndef PUBLICSTRINGMACRO_H
#define PUBLICSTRINGMACRO_H

#define ENUM_2_STR(enum_val) #enum_val
#define CLASSNAME_2_STR(className) #className

#define CASE_BRANCH_ENUM_TO_STRING(enum_val) \
case enum_val:                             \
      return #enum_val

#define STRINGIFY__LINE__CORE(lineNoLiteral) #lineNoLiteral
#define STRINGIFY__LINE__(lineNoMarco) STRINGIFY__LINE__CORE(lineNoMarco)
#define CSTRING_WITH__LINE__(specifiedCString, lineNoMarco) specifiedCString STRINGIFY__LINE__(lineNoMarco)

#endif // PUBLICSTRINGMACRO_H
