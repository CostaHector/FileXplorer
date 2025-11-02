#include "PublicVariable.h"
#include <QDir>
#include <QSet>

namespace SystemPath {
const QString& HOME_PATH() {
  static const QString path = QDir::homePath();
  return path;
}
} // namespace SystemPath

namespace TYPE_FILTER {

using DOT_EXT_HASH_SET = QSet<QString>;
bool isDotExtVideo(const QString& dotExt) {
  static const DOT_EXT_HASH_SET vidDotExtHashSet{[]() -> DOT_EXT_HASH_SET {
    DOT_EXT_HASH_SET initHashSet;
    for (const QString& asteriskDotType : VIDEO_TYPE_SET) {
      initHashSet.insert(asteriskDotType.midRef(1).toString());
    }
    return initHashSet;
  }()};
  return vidDotExtHashSet.contains(dotExt);
}
bool isDotExtImage(const QString& dotExt) {
  static const DOT_EXT_HASH_SET imgDotExtHashSet{[]() -> DOT_EXT_HASH_SET {
    DOT_EXT_HASH_SET initHashSet;
    for (const QString& asteriskDotType : IMAGE_TYPE_SET) {
      initHashSet.insert(asteriskDotType.midRef(1).toString());
    }
    return initHashSet;
  }()};
  return imgDotExtHashSet.contains(dotExt);
}

bool isDotExtCompressed(const QString& dotExt) {
  static const DOT_EXT_HASH_SET compressedDotExtHashSet{[]() -> DOT_EXT_HASH_SET {
    DOT_EXT_HASH_SET initHashSet;
    for (const QString& asteriskDotType : BUILTIN_COMPRESSED_TYPE_SET) {
      initHashSet.insert(asteriskDotType.midRef(1).toString());
    }
    return initHashSet;
  }()};
  return compressedDotExtHashSet.contains(dotExt);
}

} // namespace TYPE_FILTER
