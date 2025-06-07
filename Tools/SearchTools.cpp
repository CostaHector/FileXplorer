#include "SearchTools.h"

namespace SearchTools {
QHash<QString, SEARCH_MODE> RevertKeyValuePair(const QString (&strArr)[(unsigned char)SEARCH_MODE::MODE_BUTT]) {
  QHash<QString, SEARCH_MODE> ans;
  for (int index = 0; index < (unsigned char)SEARCH_MODE::MODE_BUTT; ++index) {
    ans[strArr[index]] = static_cast<SEARCH_MODE>(index);
  }
  return ans;
}

static const QString SEARCHMODE_ENUM2STR[(unsigned char)SEARCH_MODE::MODE_BUTT]{"Normal", "Regex", "File Contents"};

static const QHash<QString, SEARCH_MODE> SEARCHMODE_STR2ENUM{RevertKeyValuePair(SEARCHMODE_ENUM2STR)};

SEARCH_MODE GetSearchModeEnum(const QString& searchMode) {
  auto it = SEARCHMODE_STR2ENUM.constFind(searchMode);
  if (it == SEARCHMODE_STR2ENUM.constEnd()) {
    return SEARCH_MODE::MODE_BUTT;
  }
  return it.value();
}

SEARCH_MODE GetSearchModeEnum(unsigned char searchMode) {
  if (searchMode >= (unsigned char)SEARCH_MODE::MODE_BUTT) {
    return SEARCH_MODE::MODE_BUTT;
  }
  return static_cast<SEARCH_MODE>(searchMode);
}

QString GetSearchModeStr(SEARCH_MODE searchMode) {
  if (searchMode >= SEARCH_MODE::MODE_BUTT) {
    return "";
  }
  return SEARCHMODE_ENUM2STR[(unsigned char)searchMode];
}

QString GetSearchModeStr(unsigned char searchMode) {
  return GetSearchModeStr(GetSearchModeEnum(searchMode));
}

QString GetDefaultSearchModeStr() {
  return GetSearchModeStr(SEARCH_MODE::REGEX);
}
};  // namespace SearchTools
