#ifndef TABLEFIELDS_H
#define TABLEFIELDS_H

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
// 使用一个函数来返回枚举项的字符串表示
const char* GetFILEDString(FIELD_E enum_val);
}  // namespace DEV_DRV_TABLE

#define VOLUME_ENUM_TO_COLON_STRING(enum_val) ":" #enum_val

namespace DB_HEADER_KEY {
enum FIELD_E {
  Name = 0,      //
  Size,          //
  Type,          //
  DateModified,  //
  Performers,    //
  Tags,          //
  Rate,          //
  Driver,        //
  Prepath,       //
  Extra,         //
  ForSearch,     //
  BUTT
};
}  // namespace DB_HEADER_KEY

namespace MainKey {
enum FIELD_E {
  Name = 0,      //
  Size,          //
  Type,          //
  DateModified,  //
};
}

#endif  // TABLEFIELDS_H
