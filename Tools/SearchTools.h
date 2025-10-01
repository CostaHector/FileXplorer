#ifndef SEARCHTOOLS_H
#define SEARCHTOOLS_H

#include <QString>
#include "PublicMacro.h"

#define SEARCH_MODE_ENUM_VALUE_MAPPING \
  SEARCH_MODE_TYPE_ITEM(NORMAL, 0)     \
  SEARCH_MODE_TYPE_ITEM(REGEX, 1)      \
  SEARCH_MODE_TYPE_ITEM(FILE_CONTENTS, 2)

namespace SearchTools {
enum class SearchModeE {
  MODE_BEGIN = 0,
// add after BEGIN
#define SEARCH_MODE_TYPE_ITEM(enu, val) enu = val,
  SEARCH_MODE_ENUM_VALUE_MAPPING
#undef SEARCH_MODE_TYPE_ITEM
      // add before BUTT
      MODE_BUTT,
};

constexpr SearchModeE DEFAULT_SEARCH_MODE = SearchModeE::NORMAL;

inline const char* c_str(SearchModeE searchMode) {
  if (searchMode < SearchModeE::MODE_BEGIN || searchMode >= SearchModeE::MODE_BUTT) {
    return "unknown search type";
  }
  static const char SEARCH_MODE_2_CHAR_ARRAY[(int)SearchModeE::MODE_BUTT][20]{
#define SEARCH_MODE_TYPE_ITEM(enu, val) ENUM_2_STR(enu),
      SEARCH_MODE_ENUM_VALUE_MAPPING
#undef SEARCH_MODE_TYPE_ITEM
  };
  return SEARCH_MODE_2_CHAR_ARRAY[(int)searchMode];
}

inline SearchModeE GetSearchModeByActionText(const QString& actionText) {
#define SEARCH_MODE_TYPE_ITEM(enu, val) \
  if (actionText == ENUM_2_STR(enu)) {  \
    return SearchModeE::enu;            \
  }
  SEARCH_MODE_ENUM_VALUE_MAPPING
#undef SEARCH_MODE_TYPE_ITEM
  return SearchModeE::MODE_BEGIN;
}
}  // namespace SearchTools

#endif  // SEARCHTOOLS_H
