#ifndef MOUNTPATHTABLENAMEMAPPERMOCK_H
#define MOUNTPATHTABLENAMEMAPPERMOCK_H

#include <QString>

namespace MountPathTableNameMapperMock {
QString invokeToTableName(const QString& mountPath);
QString invokeToMountPath(const QString& tableName, bool& bConversionOk);
}

#endif // MOUNTPATHTABLENAMEMAPPERMOCK_H
