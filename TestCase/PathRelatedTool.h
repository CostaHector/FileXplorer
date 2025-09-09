#ifndef PATHRELATEDTOOL_H
#define PATHRELATEDTOOL_H
#include <QString>
#include <QFileInfo>

inline QString TestCaseRootPath() {
  return QFileInfo(__FILE__).absolutePath();
}
#endif  // PATHRELATEDTOOL_H
