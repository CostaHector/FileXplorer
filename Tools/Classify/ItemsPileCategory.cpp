#include "ItemsPileCategory.h"
#include "PublicVariable.h"

namespace ItemsPileCategory {

const T_DOT_EXT_2_TYPE& GetTypeFromDotExtension() {
  static const T_DOT_EXT_2_TYPE dotExt2Type{[]()->T_DOT_EXT_2_TYPE{
    T_DOT_EXT_2_TYPE initMap;
    using namespace TYPE_FILTER;
    for (const QString& asteriskDotType: VIDEO_TYPE_SET) {
      initMap[asteriskDotType.midRef(1).toString()] = VID;
    }
    for (const QString& asteriskDotType: IMAGE_TYPE_SET) {
      initMap[asteriskDotType.midRef(1).toString()] = IMG;
    }
    for (const QString& asteriskDotType: JSON_TYPE_SET) {
      initMap[asteriskDotType.midRef(1).toString()] = JSON;
    }
    return initMap;
  }()};
  return dotExt2Type;
}

} // namespace ItemsPileCategory
