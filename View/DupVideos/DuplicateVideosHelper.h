#ifndef DUPVIDEOSHELPER_H
#define DUPVIDEOSHELPER_H

#include <QString>
#include "PublicMacro.h"

struct DupVidMetaInfo {
  QString name;
  qint64 sz;
  int dur;
  qint64 modifiedDate;
  QString abspath;
  QString hash;
  bool exist;
  bool operator==(const DupVidMetaInfo& rhs) const { return abspath == rhs.abspath && hash == rhs.hash && sz == rhs.sz && dur == rhs.dur && name == rhs.name && modifiedDate == rhs.modifiedDate && exist == rhs.exist; }
};
using DupVidMetaInfoList = QList<DupVidMetaInfo>;

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

namespace RedundantVideoTool {
enum class DIFFER_BY_TYPE {
  // add after BEGIN
  BEGIN = 0,
#define DECIDE_BY_ITEM(enu, val) enu = val,
  REDUNDANT_VIDEO_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
      // add before BUTT
      BOTTOM_SIZE_INVALID,
};
static constexpr DIFFER_BY_TYPE DEFAULT_VID_DECIDE_BY = DIFFER_BY_TYPE::SIZE;

inline const char* c_str(DIFFER_BY_TYPE decideBy) {
  if (decideBy < DIFFER_BY_TYPE::BEGIN || decideBy >= DIFFER_BY_TYPE::BOTTOM_SIZE_INVALID) {
    return "unknown decideBy";
  }
  static const char decideBy2CharArray[(int)DIFFER_BY_TYPE::BOTTOM_SIZE_INVALID][20]{
#define DECIDE_BY_ITEM(enu, val) ENUM_2_STR(enu),
      REDUNDANT_VIDEO_ENUM_VALUE_MAPPING
#undef DECIDE_BY_ITEM
  };
  return decideBy2CharArray[(int)decideBy];
}
}  // namespace RedundantVideoTool

constexpr int INVALID_LEFT_SELECTED_ROW = -1;

using GroupedDupVidList = QList<DupVidMetaInfoList>;
using GroupedDupVidListArr = GroupedDupVidList[(int)RedundantVideoTool::DIFFER_BY_TYPE::BOTTOM_SIZE_INVALID];

#endif  // DUPVIDEOSHELPER_H
