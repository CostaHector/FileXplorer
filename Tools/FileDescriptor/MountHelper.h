#ifndef MOUNTHELPER_H
#define MOUNTHELPER_H

#include <QString>
#include <QStringList>

namespace MountHelper {
typedef QMap<QString, QSet<QString>> GUID_2_PNTS_SET;

constexpr char GUID_HYPEN = '-', TABLE_UNDERSCORE = '_';
// GUID_LEN|ROOTPATH
constexpr int GUID_LEN = 36;
constexpr char JOINER_STR{'|'};
constexpr int ROOTPATH_START = GUID_LEN + 1;
const QString VOLUME_NAME_TEMPLATE{R"(\\?\Volume{%1}\)"};
const char VOLUME_PREFIX[] = R"(\\?\Volume{)";
constexpr int START_INT = sizeof(VOLUME_PREFIX) - 1;

QString ExtractGuidFromVolumeName(const QString& volume);
QString ExtractGuidFromVolumeName(const wchar_t* p2volume);

QString findVolumeGuidByLabel(const QString& label);
bool isVolumeAvailable(const QString& volumeGuid);
QSet<QString> GetMountPointsByVolumeName(const wchar_t* volumeName);
QMap<QString, QSet<QString>> Volumes2ContainedMountPnts();

QString resolveFilePath(const QString& relativePath, const QString& volumeGuid);
QMap<QString, QString> GetGuid2LabelMap();
bool GetVolumeInfo(const QString& path, QString& volName);
bool GetGuidByDrive(const QString& driveStr, QString& guid);
QString FindRootByGUIDWin(const QString& targetGuid);
QString GetDisplayNameByGuidTableName(QString guidTableName);
QMap<QString, QString> GetGuidTableName2DisplayName();
QStringList GetGuidJoinDisplayName();
QString ChoppedDisplayName(const QString& guidJoinDisplayName);

GUID_2_PNTS_SET& Guids2MntPntSet(bool forceRefresh = false);
#ifdef RUNNING_UNIT_TESTS
GUID_2_PNTS_SET& MockGuids2MntPntSet();
#endif

}  // namespace MountHelper

namespace MountPathTableNameMapper {
QString toTableName(const QString& mountPath, bool* bConversionOk = nullptr);
QString toMountPath(const QString& tableName, bool* bConversionOk = nullptr);
QStringList CandidateTableNamesList();
}  // namespace MountPathTableNameMapper

#endif  // MOUNTHELPER_H
