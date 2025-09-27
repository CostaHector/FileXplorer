#ifndef PREVIEWTYPETOOL_H
#define PREVIEWTYPETOOL_H

#include "PublicMacro.h"

#define PREVIEW_TYPES_ENUM_VALUE_MAPPING \
  PREVIEW_TYPE_ITEM(NONE, 0) \
  PREVIEW_TYPE_ITEM(CATEGORY, 1) \
  PREVIEW_TYPE_ITEM(PROGRESSIVE_LOAD, 2) \
  PREVIEW_TYPE_ITEM(CAROUSEL, 3)

// PreviewTypeTool begin
namespace PreviewTypeTool {

enum class PREVIEW_TYPE_E {
  BEGIN_DEFAULT = 0,
// add after BEGIN
#define PREVIEW_TYPE_ITEM(enu, val) enu = val,
  PREVIEW_TYPES_ENUM_VALUE_MAPPING
#undef PREVIEW_TYPE_ITEM
      // add before BUTT
      END_INVALID,
};

inline const char* c_str(PREVIEW_TYPE_E viewType) {
  if (viewType < PREVIEW_TYPE_E::BEGIN_DEFAULT || viewType >= PREVIEW_TYPE_E::END_INVALID) {
    return "unknown preview type";
  }
  static constexpr const char* PREVIEW_TYPE_2_CHAR_ARRAY[(int) PREVIEW_TYPE_E::END_INVALID]{
#define PREVIEW_TYPE_ITEM(enu, val) ENUM_2_STR(enu),
      PREVIEW_TYPES_ENUM_VALUE_MAPPING
#undef PREVIEW_TYPE_ITEM
  };
  return PREVIEW_TYPE_2_CHAR_ARRAY[(int) viewType];
}
} // namespace PreviewTypeTool
#endif // PREVIEWTYPETOOL_H
