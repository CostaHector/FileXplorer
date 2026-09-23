#ifndef MOVIEDBMODELFIELD_H
#define MOVIEDBMODELFIELD_H

namespace MovieDBModelField {
enum FIELD_E {
  SampleMD5 = 0,//
  PrePathLeft,  //
  PrePathRight, //
  Name,         //
  Size,         //
  Duration,     //
  Studio,       //
  Cast,         //
  Tags,         //
  Rate,         //
  Detail,       //
  PathHash,     //
  InLocal,      //
  BUTT
};

#define SCAN_FILE_TYPE_KEY_MAPPING \
SCAN_FILE_TYPE_ITEM(VIDEOS, 0) \
SCAN_FILE_TYPE_ITEM(JSONS, 1) \

enum class ScanFilesTypeE {
#define SCAN_FILE_TYPE_ITEM(enu, enuValue) enu = enuValue,
  SCAN_FILE_TYPE_KEY_MAPPING
#undef SCAN_FILE_TYPE_ITEM
};

inline const char* ScanFilesType2Str(ScanFilesTypeE scanFileType) {
  switch (scanFileType) {
#define SCAN_FILE_TYPE_ITEM(enu, enuValue) case ScanFilesTypeE::enu: return #enu;
    SCAN_FILE_TYPE_KEY_MAPPING
#undef SCAN_FILE_TYPE_ITEM
    default:
      return "";
  }
}

#undef SCAN_FILE_TYPE_KEY_MAPPING
}

#endif // MOVIEDBMODELFIELD_H
