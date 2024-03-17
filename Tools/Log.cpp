#include "Log.h"

bool Log::color_array_inited = false;
QColor Log::COLOR_ARRAY[26] = {{0x00, 0x00, 0x00}};
const QStringList Log::INDEX2COLUMNNAME = {"time", "level", "msg", "fileName", "lineNo", "funcName"};

Log Log::fromLogFilesLine(const QString& line) {
  const auto& fields = line.split('\t');
  bool isSuccess = fields.size() >= Log::INDEX2COLUMNNAME.size();
  const int lineNo = fields[4].toInt(&isSuccess);
  QChar level = fields[1].front();
  return isSuccess ? Log{fields[0], level, fields[2], fields[3], lineNo, fields[5]} : Log();
}
