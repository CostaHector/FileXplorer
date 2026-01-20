#ifndef MOUNTPATHTABLENAMEMAPPERMOCK_H
#define MOUNTPATHTABLENAMEMAPPERMOCK_H

#include <QString>

namespace MountPathTableNameMapperMock {
QString invokeToTableName(const QString& mountPath, bool* bConversionOk = nullptr);
QString invokeToMountPath(const QString& tableName, bool* bConversionOk = nullptr);
}

#endif // MOUNTPATHTABLENAMEMAPPERMOCK_H
