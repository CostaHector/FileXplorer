#ifndef SEARCHTOOLS_H
#define SEARCHTOOLS_H

#include <QString>
#include <QHash>

namespace SearchTools {

enum class SEARCH_MODE : unsigned char {
  MODE_BEGIN = 0,
  NORMAL = MODE_BEGIN,
  REGEX,
  FILE_CONTENTS,
  MODE_BUTT,
};

SEARCH_MODE GetSearchModeEnum(const QString& searchMode);
SEARCH_MODE GetSearchModeEnum(unsigned char searchMode);
QString GetSearchModeStr(SEARCH_MODE searchMode);
QString GetSearchModeStr(unsigned char searchMode);
QString GetDefaultSearchModeStr();
}  // namespace SearchTools

#endif  // SEARCHTOOLS_H
