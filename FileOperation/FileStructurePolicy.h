#ifndef FILESTRUCTUREPOLICY_H
#define FILESTRUCTUREPOLICY_H

#include "PublicMacro.h"

#define FILE_STRUCTURE_MODE_FIELD_MAPPING \
  FILE_STRUCTURE_MODE_ITEM(QUERY, 0)      \
  FILE_STRUCTURE_MODE_ITEM(PRESERVE, 1)   \
  FILE_STRUCTURE_MODE_ITEM(FLATTEN, 2)

namespace FileStructurePolicy {

enum class FileStuctureModeE {  //
  BEGIN = 0,                      //
#define FILE_STRUCTURE_MODE_ITEM(enu, val) enu = val,
  FILE_STRUCTURE_MODE_FIELD_MAPPING
#undef FILE_STRUCTURE_MODE_ITEM
      BUTT
};

constexpr FileStuctureModeE DEFAULT_FILE_STRUCTURE_MODE = FileStuctureModeE::QUERY;

inline const char* c_str(FileStuctureModeE structureMode) {
  if (structureMode < FileStuctureModeE::BEGIN || structureMode >= FileStuctureModeE::BUTT) {
    return "unknown structure Mode";
  }
  static constexpr const char* PREVIEW_TYPE_2_CHAR_ARRAY[(int) FileStuctureModeE::BUTT]{
#define FILE_STRUCTURE_MODE_ITEM(enu, val) ENUM_2_STR(enu),
      FILE_STRUCTURE_MODE_FIELD_MAPPING
#undef FILE_STRUCTURE_MODE_ITEM
  };
  return PREVIEW_TYPE_2_CHAR_ARRAY[(int) structureMode];
}

}  // namespace FileStructurePolicy

#endif  // FILESTRUCTUREPOLICY_H
