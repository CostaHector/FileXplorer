#ifndef BYTESRANGETOOL_H
#define BYTESRANGETOOL_H

#include "PublicMacro.h"

#define BYTES_RANGE_ENUM_VALUE_MAPPING \
BYTES_RANGE_ITEM(FIRST_8,  0, 8)    \
    BYTES_RANGE_ITEM(FIRST_16,  1, 16)    \
    BYTES_RANGE_ITEM(ENTIRE_FILE, 2, -1)    \

    namespace BytesRangeTool {
  enum class BytesRangeE{
    // add after BEGIN
    BEGIN = 0,
#define BYTES_RANGE_ITEM(enu, val, bytes) enu = val,
    BYTES_RANGE_ENUM_VALUE_MAPPING
#undef BYTES_RANGE_ITEM
        // add before BUTT
        END_INVALID,
  };
  static constexpr BytesRangeE DEFAULT_BYTE_RANGE = BytesRangeE::FIRST_8;

  inline const char* c_str(BytesRangeE byteRange) {
    if (byteRange < BytesRangeE::BEGIN || byteRange >= BytesRangeE::END_INVALID) {
      return "unknown byteRange";
    }
    static const char byteRange2CharArray[(int)BytesRangeE::END_INVALID][20]{
#define BYTES_RANGE_ITEM(enu, val, bytes) ENUM_2_STR(enu),
        BYTES_RANGE_ENUM_VALUE_MAPPING
#undef BYTES_RANGE_ITEM
    };
    return byteRange2CharArray[(int)byteRange];
  }

  inline int toBytesValue(BytesRangeE byteRange) {
    static constexpr int BYTES_RANGE_2_VALUE[(int)BytesRangeE::END_INVALID]{
#define BYTES_RANGE_ITEM(enu, val, bytes) bytes,
        BYTES_RANGE_ENUM_VALUE_MAPPING
#undef BYTES_RANGE_ITEM
    };

    if (byteRange < BytesRangeE::BEGIN || byteRange >= BytesRangeE::END_INVALID) {
      return BYTES_RANGE_2_VALUE[(int)DEFAULT_BYTE_RANGE];
    }
    return BYTES_RANGE_2_VALUE[(int)byteRange];
  }
}


#endif // BYTESRANGETOOL_H
