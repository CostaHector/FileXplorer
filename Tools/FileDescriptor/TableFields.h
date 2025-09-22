#ifndef TABLEFIELDS_H
#define TABLEFIELDS_H

#include <QStringList>
namespace DEV_DRV_TABLE {
enum FIELD_E {
  ROOT_PATH = 0,  //
  VOLUME_LABEL,   //
  TOTAL_BYTES,    //
  AVAIL_BYTES,    //
  GUID,           //
  MOUNT_POINT,    //
  ADT_TIME,       //
  FILED_BUTT
};
const char* GetFILEDString(FIELD_E enum_val);
}  // namespace DEV_DRV_TABLE

namespace MainKey {
enum FIELD_E {
  Name = 0,      //
  Size,          //
  Type,          //
  DateModified,  //
};
}

namespace MOVIE_TABLE {
enum FIELD_E {
  Fd = 0,        //
  PrePathLeft,   //
  PrePathRight,  //
  Name,          //
  Size,          //
  Duration,      //
  Studio,        //
  Cast,          //
  Tags,          //
  PathHash,       //
  BUTT
};
}

namespace TORRENTS_DB_HEADER_KEY {
enum FIELD_E {
  Name = 0,
  Size,
  DateModified,
  MD5,
  PrePath,
  BUTT,
};
}  // namespace TORRENTS_DB_HEADER_KEY


namespace PERFORMER_DB_HEADER_KEY {
#define PERFORMER_TABLE_FIELD_MAPPING    \
PERFORMER_TABLE_KEY_ITEM(Name, 0)      \
    PERFORMER_TABLE_KEY_ITEM(Rate, 1)      \
    PERFORMER_TABLE_KEY_ITEM(AKA, 2)       \
    PERFORMER_TABLE_KEY_ITEM(Tags, 3)      \
    PERFORMER_TABLE_KEY_ITEM(Ori, 4)       \
    PERFORMER_TABLE_KEY_ITEM(Vids, 5)      \
    PERFORMER_TABLE_KEY_ITEM(Imgs, 6)      \
    PERFORMER_TABLE_KEY_ITEM(Detail, 7)    \

    enum FIELD_E {
#define PERFORMER_TABLE_KEY_ITEM(enu, val) enu = val,
      PERFORMER_TABLE_FIELD_MAPPING
#undef PERFORMER_TABLE_KEY_ITEM
    };
const char* GetFILEDString(FIELD_E enum_val);
extern const QStringList DB_HEADER;
}  // namespace PERFORMER_DB_HEADER_KEY

namespace VidDupHelper{
#define VID_DUP_TABLE_FIELD_MAPPING    \
VID_DUP_TABLE_KEY_ITEM(EFFECTIVE_NAME, 0)      \
VID_DUP_TABLE_KEY_ITEM(SIZE, 1)      \
VID_DUP_TABLE_KEY_ITEM(DURATION, 2)       \
VID_DUP_TABLE_KEY_ITEM(DATE, 3)      \
VID_DUP_TABLE_KEY_ITEM(ABSOLUTE_PATH, 4)       \
VID_DUP_TABLE_KEY_ITEM(FIRST_1024_HASH, 5)      \
VID_DUP_TABLE_KEY_ITEM(FIRST_8192_HASH, 6)      \
VID_DUP_TABLE_KEY_ITEM(FULL_SIZE_HASH, 7)    \

    enum FIELD_E {
#define VID_DUP_TABLE_KEY_ITEM(enu, val) enu = val,
      VID_DUP_TABLE_FIELD_MAPPING
#undef VID_DUP_TABLE_KEY_ITEM
    };
}

#endif  // TABLEFIELDS_H
