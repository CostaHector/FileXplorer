#ifndef JSON_MGR_H
#define JSON_MGR_H

#include "JsonPr.h"
#include <QList>

struct JsonMgr {
  int ReadADirectory(const QString& path);
  QList<JsonPr> mCachedJsons;
};

#endif
