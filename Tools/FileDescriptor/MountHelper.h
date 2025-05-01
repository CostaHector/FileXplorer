#ifndef MOUNTHELPER_H
#define MOUNTHELPER_H

#include <QString>

namespace MountHelper
{
QString findVolumeGuidByLabel(const QString& label);
bool isVolumeAvailable(const QString& volumeGuid);
bool MountVolume(const QString& volumeGuid, const QString& label, QString& volMountPoint);
bool UnmountVolume(const QString& mountPath);
QString resolveFilePath(const QString& relativePath, const QString& volumeGuid);
QMap<QString, QString> GetGuid2LabelMap();
bool GetVolumeInfo(const QString &path, QString& volName);
bool GetGuidByDrive(const QString& driveStr, QString& guid);
bool IsAdministrator();
bool RunAsAdmin();
}

#endif // MOUNTHELPER_H
