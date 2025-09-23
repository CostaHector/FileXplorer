#ifndef REDUNDANT_IMAGE_TYPE_H
#define REDUNDANT_IMAGE_TYPE_H

#include "PublicMacro.h"

#define REDUNDANT_IMAGE_ENUM_VALUE_MAPPING \
  DECIDE_BY_ITEM(LIBRARY, 0)               \
  DECIDE_BY_ITEM(MD5, 1)

namespace RedundantImageTool {
enum class DecideByE {
  // add after BEGIN
  BEGIN = 0,
#define DECIDE_BY_ITEM(enu, val) enu = val,
  REDUNDANT_IMAGE_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
      // add before BUTT
      END_INVALID,
};
static constexpr DecideByE DEFAULT_DECIDE_BY = DecideByE::LIBRARY;

inline const char* c_str(DecideByE decideBy) {
  if (decideBy < DecideByE::BEGIN || decideBy >= DecideByE::END_INVALID) {
    return "unknown decideBy";
  }
  static const char decideBy2CharArray[(int)DecideByE::END_INVALID][20]{
#define DECIDE_BY_ITEM(enu, val) ENUM_2_STR(enu),
      REDUNDANT_IMAGE_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
  };
  return decideBy2CharArray[(int)decideBy];
}
}  // namespace RedundantImageTool

#endif  // REDUNDANT_IMAGE_TYPE_H
