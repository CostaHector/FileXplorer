#ifndef PATHRELATEDTOOL_H
#define PATHRELATEDTOOL_H
#include <QFileInfo>

QString TestCaseRootPath() {
  return QFileInfo(__FILE__).absolutePath();
}

#endif  // PATHRELATEDTOOL_H
