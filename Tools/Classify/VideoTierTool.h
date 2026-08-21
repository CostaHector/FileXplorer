#ifndef VIDEOTIERTOOL_H
#define VIDEOTIERTOOL_H

#include <QString>

/* enumName, enumValue, enumString, [rateSpanBegin, rateSpanEnd)*/
#define VIDEO_TIER_KEY_MAPPING \
VIDEO_TIER_KEY_ITEM(ARCHIVAL, 0, "Archival", 10, 11) \
    VIDEO_TIER_KEY_ITEM(ESSENTIALS, 1, "Essentials", 8, 10) \
    VIDEO_TIER_KEY_ITEM(NORMALS, 2, "Normal", 6, 8) \
    VIDEO_TIER_KEY_ITEM(CASUALS, 3, "Casuals", 3, 6) \
    VIDEO_TIER_KEY_ITEM(DISPOSABLE, 4, "Disposable", 1, 3) \
    VIDEO_TIER_KEY_ITEM(BUTT_INVALID, 5, "ButtInvalid", 0, 1) \

    ;

namespace VideoTierTool {
enum class VideoTierE {
  BEGIN = 0,
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd) enumName = enumValue,
  VIDEO_TIER_KEY_MAPPING
#undef VIDEO_TIER_KEY_ITEM
};

inline bool isVideoTierValid(int videoTier) {
  return (int)VideoTierE::BEGIN <= videoTier && videoTier < (int)VideoTierE::BUTT_INVALID;
}

inline bool isVideoTierValid(VideoTierE videoTier) {
  return videoTier != VideoTierE::BUTT_INVALID;
}

inline const char* c_str(VideoTierE videoTier) {
  switch (videoTier) {
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd) case VideoTierE::enumName: return enumString;
    VIDEO_TIER_KEY_MAPPING
#undef VIDEO_TIER_KEY_ITEM
        default:
                  return "ButtInvalid";
  }
}

inline VideoTierE MovieRate2VideoTierE(int movieRate) {
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd) if (rateSpanBegin<=movieRate && movieRate<rateSpanEnd) { return VideoTierE::enumName;}
  VIDEO_TIER_KEY_MAPPING
#undef VIDEO_TIER_KEY_ITEM
      return VideoTierE::BUTT_INVALID;
}

inline QString GetArchiveToFolderName(QString parentFolderName, VideoTierE videoTier) {
  return parentFolderName + " " + c_str(videoTier);
}

}

#undef VIDEO_TIER_KEY_MAPPING

#endif // VIDEOTIERTOOL_H
