#ifndef MOUNTHELPER_H
#define MOUNTHELPER_H

#include <QString>
#include <QStringList>

namespace MountPathTableNameMapper {
bool isMountPointOnline(const QString& mountPoint);
QString toTableName(const QString& mountPath);
QString toMountPath(const QString& tableName, bool& bConversionOk);
QStringList CandidateTableNamesList();
}  // namespace MountPathTableNameMapper

#endif  // MOUNTHELPER_H
