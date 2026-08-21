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

enum class ScanFilesTypeE {
  VIDEOS = 0,
  JSONS,
};

}

#endif // MOVIEDBMODELFIELD_H
