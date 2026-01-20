#ifndef DUPLICATE_IMAGES_HELPER_H
#define DUPLICATE_IMAGES_HELPER_H

#include <QString>
#include "PublicMacro.h"

#define DUP_IMAGE_META_INFO_KEY_MAPPING \
  DUP_IMAGE_META_INFO_KEY_ITEM(Name, 0, QString, DataFormatter::formatDefault) \
  DUP_IMAGE_META_INFO_KEY_ITEM(Size, 1, qint64, DataFormatter::formatFileSizeGMKB) \
  DUP_IMAGE_META_INFO_KEY_ITEM(MD5, 2, QByteArray, DataFormatter::formatDefault) \
  DUP_IMAGE_META_INFO_KEY_ITEM(AbsPath, 3, QString, DataFormatter::formatDefault)

namespace DuplicateImageMetaInfo {
enum DIColumnE {
#define DUP_IMAGE_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) enu = enumVal,
  DUP_IMAGE_META_INFO_KEY_MAPPING
#undef DUP_IMAGE_META_INFO_KEY_ITEM
};

constexpr const char* DI_TABLE_HEADERS[]{
#define DUP_IMAGE_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) #enu,
    DUP_IMAGE_META_INFO_KEY_MAPPING
#undef DUP_IMAGE_META_INFO_KEY_ITEM
};
constexpr int DI_TABLE_HEADERS_COUNT = sizeof(DI_TABLE_HEADERS) / sizeof(DI_TABLE_HEADERS[0]);

struct REDUNDANT_IMG_INFO {
#define DUP_IMAGE_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) VariableType m_##enu;
  DUP_IMAGE_META_INFO_KEY_MAPPING
#undef DUP_IMAGE_META_INFO_KEY_ITEM
  bool operator==(const REDUNDANT_IMG_INFO& rhs) const {
    return m_Name == rhs.m_Name && m_Size == rhs.m_Size && m_AbsPath == rhs.m_AbsPath;
  }
};

using RedundantImagesList = QList<REDUNDANT_IMG_INFO>;
} // namespace DuplicateImageMetaInfo

#define REDUNDANT_IMAGE_ENUM_VALUE_MAPPING \
  DECIDE_BY_ITEM(LIBRARY, 0) \
  DECIDE_BY_ITEM(MD5, 1)

namespace DuplicateImageDetectionCriteria {
enum class DICriteriaE {
  // add after BEGIN
  BEGIN = 0,
#define DECIDE_BY_ITEM(enu, val) enu = val,
  REDUNDANT_IMAGE_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
      // add before BUTT
      END_INVALID,
};
static constexpr DICriteriaE DEFAULT_DI_CRITERIA_E = DICriteriaE::LIBRARY;

inline const char* c_str(DICriteriaE diCriteriaE) {
  if (diCriteriaE < DICriteriaE::BEGIN || diCriteriaE >= DICriteriaE::END_INVALID) {
    return "unknown di criteria";
  }
  static constexpr const char* DI_CRITERIA_E_2_CHAR_ARRAY[(int) DICriteriaE::END_INVALID]{
#define DECIDE_BY_ITEM(enu, val) ENUM_2_STR(enu),
      REDUNDANT_IMAGE_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
  };
  return DI_CRITERIA_E_2_CHAR_ARRAY[(int) diCriteriaE];
}
} // namespace DuplicateImageDetectionCriteria

#endif // DUPLICATE_IMAGE_DETECTION_CRITERIA_H
