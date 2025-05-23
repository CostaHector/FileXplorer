#include "PathRelatedTool.h"
#include <QFileInfo>

QString TestCaseRootPath() {
  return QFileInfo(__FILE__).absolutePath();
}

