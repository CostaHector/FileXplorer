#ifndef MOUNTHELPER_H
#define MOUNTHELPER_H

#include <QString>

namespace MountHelper {

constexpr QChar JOINER_STR{'|'};
constexpr QChar GUID_HYPEN = '-', TABLE_UNDERSCORE = '_';
constexpr int GUID_LEN = 36;
const QString VOLUME_NAME_TEMPLATE{R"(\\?\Volume{%1}\)"};
const char VOLUME_PREFIX[] = R"(\\?\Volume{)";
constexpr int START_INT = sizeof(VOLUME_PREFIX) - 1;

QString ExtractGuidFromVolumeName(const QString& volume);
QString ExtractGuidFromVolumeName(const wchar_t* p2volume);

QString findVolumeGuidByLabel(const QString& label);
bool isVolumeAvailable(const QString& volumeGuid);
bool MountVolume(const QString& volumeGuid, const QString& label, QString& volMountPoint);
bool UnmountVolume(const QString& volMountPoint);
QSet<QString> GetMountPointsByVolumeName(const wchar_t* volumeName);
QMap<QString, QSet<QString>> Volumes2ContainedMountPnts();

QString resolveFilePath(const QString& relativePath, const QString& volumeGuid);
QMap<QString, QString> GetGuid2LabelMap();
bool GetVolumeInfo(const QString& path, QString& volName);
bool GetGuidByDrive(const QString& driveStr, QString& guid);
bool IsAdministrator();
bool RunAsAdmin();

QString FindRootByGUIDWin(const QString& targetGuid);
typedef QMap<QString, QSet<QString>> GUID_2_PNTS_SET;
const GUID_2_PNTS_SET& Guids2MntPntSet(bool forceRefresh = false);
QString GetDisplayNameByGuidTableName(QString guidTableName);
QMap<QString, QString> GetGuidTableName2DisplayName();
QStringList GetGuidJoinDisplayName();
QString ChoppedDisplayName(const QString& GuidJoinDisplayName);
}  // namespace MountHelper

#endif  // MOUNTHELPER_H
