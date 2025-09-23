#ifndef DUPLICATEVIDEOSHELPER_H
#define DUPLICATEVIDEOSHELPER_H

#include <QString>
#include "PublicMacro.h"

#define DUP_VIDEO_META_INFO_KEY_MAPPING \
  DUP_VIDEO_META_INFO_KEY_ITEM(Name, 0, QString, DataFormatter::formatDefault) \
  DUP_VIDEO_META_INFO_KEY_ITEM(Size, 1, qint64, DataFormatter::formatFileSizeGMKB) \
  DUP_VIDEO_META_INFO_KEY_ITEM(Duration, 2, int, DataFormatter::formatDateIsoMs) \
  DUP_VIDEO_META_INFO_KEY_ITEM(ModifiedDate, 3, qint64, DataFormatter::formatDateIso) \
  DUP_VIDEO_META_INFO_KEY_ITEM(AbsPath, 4, QString, DataFormatter::formatDefault) \
  DUP_VIDEO_META_INFO_KEY_ITEM(Hash, 5, QString, DataFormatter::formatDefault) \

namespace DuplicateVideoMetaInfo {
enum DVColumnE {
#define DUP_VIDEO_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) enu = enumVal,
  DUP_VIDEO_META_INFO_KEY_MAPPING
#undef DUP_VIDEO_META_INFO_KEY_ITEM
};

constexpr const char* DV_TABLE_HEADERS[] {
#define DUP_VIDEO_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) #enu,
  DUP_VIDEO_META_INFO_KEY_MAPPING
    #undef DUP_VIDEO_META_INFO_KEY_ITEM
};
constexpr int DV_TABLE_HEADERS_COUNT = sizeof(DV_TABLE_HEADERS) / sizeof(DV_TABLE_HEADERS[0]);

struct DVInfo {
#define DUP_VIDEO_META_INFO_KEY_ITEM(enu, enumVal, VariableType, formatter) VariableType m_##enu;
  DUP_VIDEO_META_INFO_KEY_MAPPING
#undef DUP_VIDEO_META_INFO_KEY_ITEM
  bool operator==(const DVInfo& rhs) const { return m_Name == rhs.m_Name && m_Size == rhs.m_Size && m_Duration == rhs.m_Duration && m_ModifiedDate == rhs.m_ModifiedDate && m_AbsPath == rhs.m_AbsPath; }
};

}
using DupVidMetaInfoList = QList<DuplicateVideoMetaInfo::DVInfo>;

// C:/DISK/F24 => C__DISK_F24
// /home/costa/Document => _HOME_COSTA_DOCUMENT
QString GetTableName(const QString& pathName);
// DISK_F24 => C:/DISK/F24
// _HOME_COSTA_DOCUMENT => /home/costa/Document
QString TableName2Path(const QString& tableName);

struct DupVidTableName2RecordCount {
  QString tableName;
  int count;
  bool operator==(const DupVidTableName2RecordCount& rhs) const { return tableName == rhs.tableName && count == rhs.count; }
  bool operator<(const DupVidTableName2RecordCount& rhs) const { return tableName < rhs.tableName || (tableName == rhs.tableName && count < rhs.count); }
};
using DupVidTableName2RecordCountList = QList<DupVidTableName2RecordCount>;

#define REDUNDANT_VIDEO_ENUM_VALUE_MAPPING \
  DECIDE_BY_ITEM(DURATION, 0)              \
  DECIDE_BY_ITEM(SIZE, 1)

namespace DuplicateVideoDetectionCriteria {
enum class DVCriteriaE {
  // add after BEGIN
  BEGIN = 0,
#define DECIDE_BY_ITEM(enu, val) enu = val,
  REDUNDANT_VIDEO_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
  // add before BUTT
  BOTTOM_SIZE_INVALID,
};
static constexpr DVCriteriaE DEFAULT_VD_CRITERIA_E = DVCriteriaE::SIZE;

inline const char* c_str(DVCriteriaE dvCriteria) {
  if (dvCriteria < DVCriteriaE::BEGIN || dvCriteria >= DVCriteriaE::BOTTOM_SIZE_INVALID) {
    return "unknown dv criteria";
  }
  static constexpr const char* DV_CRITERIA_E_2_CHAR_ARRAY[(int)DVCriteriaE::BOTTOM_SIZE_INVALID] {
#define DECIDE_BY_ITEM(enu, val) ENUM_2_STR(enu),
    REDUNDANT_VIDEO_ENUM_VALUE_MAPPING
    #undef DECIDE_BY_ITEM
  };
  return DV_CRITERIA_E_2_CHAR_ARRAY[(int)dvCriteria];
}
}  // namespace DuplicateVideoDetectionCriteria

constexpr int INVALID_LEFT_SELECTED_ROW = -1;

using GroupedDupVidList = QList<DupVidMetaInfoList>;
using GroupedDupVidListArr = GroupedDupVidList[(int)DuplicateVideoDetectionCriteria::DVCriteriaE::BOTTOM_SIZE_INVALID];

#endif  // DUPLICATEVIDEOSHELPER_H
