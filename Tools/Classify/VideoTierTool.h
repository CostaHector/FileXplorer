#ifndef VIDEOTIERTOOL_H
#define VIDEOTIERTOOL_H

#include <QString>
#include <QRgb>

/* enumName, enumValue, enumString, [rateSpanBegin, rateSpanEnd), savedIn, colorR, colorG, colorB, */
#define VIDEO_TIER_KEY_MAPPING \
VIDEO_TIER_KEY_ITEM(ARCHIVAL,     0, "Archival",     10, 11, "Local + Cloud (with PAR2)",         0xFF, 0x80, 0x00) \
    VIDEO_TIER_KEY_ITEM(ESSENTIALS,   1, "Essentials",    8, 10, "Local + Cloud",                     0xA3, 0x35, 0xEE) \
    VIDEO_TIER_KEY_ITEM(NORMALS,      2, "Normal",        6,  8, "Local + Cloud",                     0x00, 0x70, 0xDD) \
    VIDEO_TIER_KEY_ITEM(STANDBY,      3, "Standby",       5,  6, "Cloud (temporary, pending local copy)", 0x1E, 0xAA, 0x1E) \
    VIDEO_TIER_KEY_ITEM(CASUALS,      4, "Casuals",       3,  5, "Cloud only",                        0x88, 0x88, 0x88) \
    VIDEO_TIER_KEY_ITEM(DISPOSABLE,   5, "Disposable",    1,  3, "Cloud only (may be deleted)",       0x55, 0x55, 0x55) \
    VIDEO_TIER_KEY_ITEM(BUTT_INVALID, 6, "ButtInvalid",   0,  1, "Invalid",                           0x33, 0x33, 0x33)

namespace VideoTierTool {
enum class VideoTierE {
  BEGIN = 0,
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd, savedIn, colorR, colorG, colorB) enumName = enumValue,
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
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd, savedIn, colorR, colorG, colorB) case VideoTierE::enumName: return enumString;
    VIDEO_TIER_KEY_MAPPING
#undef VIDEO_TIER_KEY_ITEM
        default:
                  return "ButtInvalid";
  }
}

inline QString GetVideoTierActionToolTip(VideoTierE videoTier) {
  switch (videoTier) {
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd, savedIn, colorR, colorG, colorB) case VideoTierE::enumName: return QString("Scores in [%1, %2) are stored in %3.").arg(rateSpanBegin).arg(rateSpanEnd).arg(savedIn);
    VIDEO_TIER_KEY_MAPPING
#undef VIDEO_TIER_KEY_ITEM
        default:
          return "Invalid tier: scores outside the valid range [0, 11).";
  }
}

inline VideoTierE MovieRate2VideoTierE(int movieRate) {
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd, savedIn, colorR, colorG, colorB) if (rateSpanBegin<=movieRate && movieRate<rateSpanEnd) { return VideoTierE::enumName;}
  VIDEO_TIER_KEY_MAPPING
#undef VIDEO_TIER_KEY_ITEM
      return VideoTierE::BUTT_INVALID;
}

inline QString GetArchiveToFolderName(QString parentFolderName, VideoTierE videoTier) {
  return parentFolderName + " " + c_str(videoTier);
}

inline QRgb GetVideoTierColorRgb(VideoTierE tier) {
#define VIDEO_TIER_KEY_ITEM(enumName, enumValue, enumString, rateSpanBegin, rateSpanEnd, savedIn, colorR, colorG, colorB) \
  if (tier == VideoTierE::enumName) return qRgb(colorR, colorG, colorB);
  VIDEO_TIER_KEY_MAPPING
#undef VIDEO_TIER_KEY_ITEM
      return qRgb(0x33, 0x33, 0x33);
}

}

#undef VIDEO_TIER_KEY_MAPPING

#endif // VIDEOTIERTOOL_H
