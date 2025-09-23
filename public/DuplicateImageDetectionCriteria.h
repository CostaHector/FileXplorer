#ifndef DUPLICATE_IMAGE_DETECTION_CRITERIA_H
#define DUPLICATE_IMAGE_DETECTION_CRITERIA_H

#include "PublicMacro.h"

#define REDUNDANT_IMAGE_ENUM_VALUE_MAPPING \
  DECIDE_BY_ITEM(LIBRARY, 0)               \
  DECIDE_BY_ITEM(MD5, 1)

namespace DuplicateImageDetectionCriteria {
enum class DICriteriaE {
  // add after BEGIN
  BEGIN = 0,
#define DECIDE_BY_ITEM(enu, val) enu = val,
  REDUNDANT_IMAGE_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
      // add before BUTT
      END_INVALID,
};
static constexpr DICriteriaE DEFAULT_DI_CRITERIA_E = DICriteriaE::LIBRARY;

inline const char* c_str(DICriteriaE diCriteriaE) {
  if (diCriteriaE < DICriteriaE::BEGIN || diCriteriaE >= DICriteriaE::END_INVALID) {
    return "unknown di criteria";
  }
  static constexpr const char* DI_CRITERIA_E_2_CHAR_ARRAY[(int)DICriteriaE::END_INVALID]{
#define DECIDE_BY_ITEM(enu, val) ENUM_2_STR(enu),
      REDUNDANT_IMAGE_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
  };
  return DI_CRITERIA_E_2_CHAR_ARRAY[(int)diCriteriaE];
}
}  // namespace DuplicateImageDetectionCriteria

#endif  // DUPLICATE_IMAGE_DETECTION_CRITERIA_H
