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

int GetImageNoTimeStampPatternLength(const QString& srcFrameImgName) {
  // xxxx.mp4_20260818_155313.075.jpg <- screenshot created timestamp (Pot Player)
  constexpr int POT_PLAYER_SOURCE_FRAME_FORMAT_EXAMPLE_VALID_CHAR_CNT{sizeof("_20260818_155313.075.jpg") - 1};
  const QRegularExpression POT_PLAYER_SOURCE_FRAME_NAME_PATTERN(R"(_\d{8}_\d{6}\.\d{3}\.jpg$)");
  // xxxx_011050.075.jpg <- screenshot at video time=01:10:50.075 (Built In)
  constexpr int BUILT_IN_SOURCE_FRAME_FORMAT_EXAMPLE_VALID_CHAR_CNT{sizeof("_010203.567.jpg") - 1};
  const QRegularExpression BUILT_IN_SOURCE_FRAME_NAME_PATTERN(R"(_\d{6}\.\d{3}\.jpg$)");

  QString ans;
  if (POT_PLAYER_SOURCE_FRAME_NAME_PATTERN.match(srcFrameImgName).hasMatch()) {
    return srcFrameImgName.size() - POT_PLAYER_SOURCE_FRAME_FORMAT_EXAMPLE_VALID_CHAR_CNT;
  } else if (BUILT_IN_SOURCE_FRAME_NAME_PATTERN.match(srcFrameImgName).hasMatch()) {
    return srcFrameImgName.size() - BUILT_IN_SOURCE_FRAME_FORMAT_EXAMPLE_VALID_CHAR_CNT;
  } else {
    return -1;
  }
}

} // namespace ItemsPileCategory
