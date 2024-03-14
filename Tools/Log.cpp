#include "Log.h"

bool Log::color_array_inited = false;
QColor Log::COLOR_ARRAY[26] = {{0x00, 0x00, 0x00}};
const QStringList Log::INDEX2COLUMNNAME = {"time", "level", "msg", "fileName", "lineNo", "funcName"};
