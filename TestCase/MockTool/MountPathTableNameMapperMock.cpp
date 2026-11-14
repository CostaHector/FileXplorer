#include "MountPathTableNameMapperMock.h"

namespace MountPathTableNameMapperMock {

QString invokeToTableName(const QString& mountPath) { // "/tmp/File-xx", "C:/temp/File-xx"
  QString tableName = mountPath;
  return tableName.replace('/', '_').replace(':', '_');
}

QString invokeToMountPath(const QString& tableName, bool& bConversionOk) { // "_tmp_File_xx", "C__tmp_File-xx"
  bConversionOk = true;
  QString mountPath = tableName;
  if (mountPath.size() >= 3 && mountPath[0] == 'C' && mountPath[1] == '_' && mountPath[2] == '_') {
    mountPath[1] = ':';
  }
  return mountPath.replace('_', '/');
}

}
