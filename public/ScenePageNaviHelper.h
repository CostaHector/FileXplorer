#ifndef SCENEPAGENAVIHELPER_H
#define SCENEPAGENAVIHELPER_H

#include "PublicMacro.h"

#define SCENE_PAGE_NAVIGATION_KEY_MAPPING  \
  SCENE_PAGE_NAVIGATION_KEY_ITEM(FRONT, 0) \
  SCENE_PAGE_NAVIGATION_KEY_ITEM(PREVIOUS, 1)  \
  SCENE_PAGE_NAVIGATION_KEY_ITEM(NEXT, 2)  \
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

#endif  // SCENEPAGENAVIHELPER_H
