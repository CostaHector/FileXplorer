#ifndef SCENEPAGENAVIHELPER_H
#define SCENEPAGENAVIHELPER_H

#include "PublicMacro.h"

#define SCENE_PAGE_NAVIGATION_KEY_MAPPING     \
  SCENE_PAGE_NAVIGATION_KEY_ITEM(FRONT, 0)    \
  SCENE_PAGE_NAVIGATION_KEY_ITEM(PREVIOUS, 1) \
  SCENE_PAGE_NAVIGATION_KEY_ITEM(NEXT, 2)     \
  SCENE_PAGE_NAVIGATION_KEY_ITEM(BACK, 3)

namespace ScenePageNaviHelper {
enum class PageNaviE : int {
  BEGIN_DEFAULT = 0,
#define SCENE_PAGE_NAVIGATION_KEY_ITEM(enu, enumVal) enu = enumVal,
  SCENE_PAGE_NAVIGATION_KEY_MAPPING
#undef SCENE_PAGE_NAVIGATION_KEY_ITEM
      END_INVALID
};

inline const char* c_str(PageNaviE pageNavi) {
  if (pageNavi < PageNaviE::BEGIN_DEFAULT || pageNavi >= PageNaviE::END_INVALID) {
    return "unknown scene page navigation";
  }
  static constexpr const char* PAGE_NAVI_2_CHAR_ARRAY[(int)PageNaviE::END_INVALID]{
#define SCENE_PAGE_NAVIGATION_KEY_ITEM(enu, val) ENUM_2_STR(enu),
      SCENE_PAGE_NAVIGATION_KEY_MAPPING
#undef SCENE_PAGE_NAVIGATION_KEY_ITEM
  };
  return PAGE_NAVI_2_CHAR_ARRAY[(int)pageNavi];
}

}  // namespace ScenePageNaviHelper

#define SCENE_SORT_ORDER_KEY_MAPPING       \
  SCENE_SORT_ORDER_KEY_ITEM(MOVIE_PATH, 0) \
  SCENE_SORT_ORDER_KEY_ITEM(MOVIE_SIZE, 1) \
  SCENE_SORT_ORDER_KEY_ITEM(RATE, 2)       \
  SCENE_SORT_ORDER_KEY_ITEM(UPLOADED_TIME, 3)

namespace SceneSortOrderHelper {
enum class SortDimE : int {
  BEGIN_DEFAULT = 0,
#define SCENE_SORT_ORDER_KEY_ITEM(enu, enumVal) enu = enumVal,
  SCENE_SORT_ORDER_KEY_MAPPING
#undef SCENE_SORT_ORDER_KEY_ITEM
      END_INVALID
};

constexpr SortDimE DEFAULT_SCENE_SORT_ORDER = SortDimE::MOVIE_PATH;

inline const char* c_str(SortDimE sortOrder) {
  if (sortOrder < SortDimE::BEGIN_DEFAULT || sortOrder >= SortDimE::END_INVALID) {
    return "unknown scene sort order";
  }
  static constexpr const char* SORT_ORDER_2_CHAR_ARRAY[(int)SortDimE::END_INVALID]{
#define SCENE_SORT_ORDER_KEY_ITEM(enu, val) ENUM_2_STR(enu),
      SCENE_SORT_ORDER_KEY_MAPPING
#undef SCENE_SORT_ORDER_KEY_ITEM
  };
  return SORT_ORDER_2_CHAR_ARRAY[(int)sortOrder];
}

// from switch case
// inline SortDimE toEnum(int sortOrderInt) {
//   switch(sortOrderInt){
// #define SCENE_SORT_ORDER_KEY_ITEM(enu, val) case val: return enu;
//     SCENE_SORT_ORDER_KEY_MAPPING
// #undef SCENE_SORT_ORDER_KEY_ITEM
//     default:
//       return DEFAULT_SCENE_SORT_ORDER;
//   }
// }

inline SortDimE toEnum(int sortOrderInt) {
  static constexpr SortDimE int2Enum[(int)SortDimE::END_INVALID] {
#define SCENE_SORT_ORDER_KEY_ITEM(enu, val) SortDimE::enu,
    SCENE_SORT_ORDER_KEY_MAPPING
#undef SCENE_SORT_ORDER_KEY_ITEM
  };
  static_assert(sizeof(int2Enum) / sizeof(int2Enum[0]) == static_cast<int>(SortDimE::END_INVALID),
                "int2Enum size mismatch");
  if (sortOrderInt < (int)SortDimE::BEGIN_DEFAULT || sortOrderInt >= (int)SortDimE::END_INVALID) {
    return DEFAULT_SCENE_SORT_ORDER;
  }
  return int2Enum[sortOrderInt];
}

}  // namespace SceneSortOrderHelper

#endif  // SCENEPAGENAVIHELPER_H
