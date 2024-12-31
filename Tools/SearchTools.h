#ifndef SEARCHTOOLS_H
#define SEARCHTOOLS_H

#include <QString>
#include <QHash>

namespace SearchTools {

enum class SEARCH_MODE : unsigned char {
  MODE_BEGIN = 0,
  NORMAL = MODE_BEGIN,
  WILDCARD,
  REGEX,
  SEARCH_FOR_FILE_CONTENTS,
  MODE_BUTT,
};

SEARCH_MODE GetSearchModeEnum(const QString& searchMode);

SEARCH_MODE GetSearchModeEnum(unsigned char searchMode);

QStringList GetSearchModeStrList();
};  // namespace SearchTools

#endif  // SEARCHTOOLS_H
