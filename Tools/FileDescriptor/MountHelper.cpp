#include "MountHelper.h"
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include <QSet>
#include <QStorageInfo>
#include <QProcess>

#ifndef WIN32
namespace MountHelper{

QString ExtractGuidFromVolumeName(const QString& volume) { return {}; }
QString ExtractGuidFromVolumeName(const wchar_t* p2volume) { return {}; }

QString findVolumeGuidByLabel(const QString& label) { return {}; }
bool isVolumeAvailable(const QString& volumeGuid) { return false; }
bool MountVolume(const QString& volumeGuid, const QString& label, QString& volMountPoint) { return false; }
bool UnmountVolume(const QString& volMountPoint) { return false; }
QSet<QString> GetMountPointsByVolumeName(const wchar_t* volumeName) { return {}; }
QMap<QString, QSet<QString>> Volumes2ContainedMountPnts() { return {}; }

QString resolveFilePath(const QString& relativePath, const QString& volumeGuid) { return {}; }
QMap<QString, QString> GetGuid2LabelMap() { return {}; }
bool GetVolumeInfo(const QString& path, QString& volName) { return false; }
bool GetGuidByDrive(const QString& driveStr, QString& guid) { return false; }
bool IsAdministrator() { return false; }
bool RunAsAdmin() { return false; }

QString FindRootByGUIDWin(const QString& targetGuid) { return {}; }
const GUID_2_PNTS_SET& Guids2MntPntSet(bool forceRefresh) {
  const static GUID_2_PNTS_SET emptySet;
  return emptySet;
}
QString GetDisplayNameByGuidTableName(QString guidTableName) { return {}; }
QMap<QString, QString> GetGuidTableName2DisplayName() { return {}; }
QStringList GetGuidJoinDisplayName() { return {}; }
QString ChoppedDisplayName(const QString& guidJoinDisplayName) { return {}; }
}
#else

#include <windows.h>
#include "fileapi.h"

QString MountHelper::ExtractGuidFromVolumeName(const QString& volume) {
  // R"(\\?\Volume{36 chars}\)"
  return volume.mid(START_INT, GUID_LEN);
}

QString MountHelper::ExtractGuidFromVolumeName(const wchar_t* p2volume) {
  // R"(\\?\Volume{36 chars}\)"
  if (p2volume == nullptr) {
    qCritical("p2volume is nullptr");
    return "";
  }
  const QString volume = QString::fromStdWString(p2volume);
  return ExtractGuidFromVolumeName(volume);
}

// 动态获取GUID（示例：通过卷标匹配）
QString MountHelper::findVolumeGuidByLabel(const QString& label) {
  wchar_t volumeName[MAX_PATH]{0};
  HANDLE hFind = FindFirstVolumeW(volumeName, MAX_PATH);
  if (hFind == INVALID_HANDLE_VALUE) {
    qWarning("find first volume failed. label[%s] not exist", qPrintable(label));
    return "";
  }

  wchar_t labelBuf[MAX_PATH]{0};
  do {
    memset(labelBuf, 0, sizeof(labelBuf));
    if (GetVolumeInformationW(volumeName, labelBuf, MAX_PATH, NULL, NULL, NULL, NULL, 0)) {
      if (QString::fromWCharArray(labelBuf) == label) {
        return ExtractGuidFromVolumeName(volumeName);  // 提取GUID
      }
    }
  } while (FindNextVolumeW(hFind, volumeName, MAX_PATH));
  return "";
}

// 枚举系统中所有已连接的卷，检查目标GUID是否存在
bool MountHelper::isVolumeAvailable(const QString& dstVolumeGuid) {
  wchar_t volumeName[MAX_PATH]{0};
  HANDLE hFind = FindFirstVolumeW(volumeName, MAX_PATH);
  if (hFind == INVALID_HANDLE_VALUE) {
    qWarning("find first volume failed. volumeGuid[%s] not exist", qPrintable(dstVolumeGuid));
    return false;
  }

  do {
    const QString currentGuid{ExtractGuidFromVolumeName(volumeName)};  // 提取GUID部分
    if (currentGuid == dstVolumeGuid) {
      FindVolumeClose(hFind);
      return true;
    }
  } while (FindNextVolumeW(hFind, volumeName, MAX_PATH));

  FindVolumeClose(hFind);
  return false;
}

bool MountHelper::RunAsAdmin() {
  const QString& program = "path\\to\\yourapp.exe";  // 替换为你的应用程序路径
  QString runas{"runas"};
  QStringList cmds{"/user:administrator", QString(R"(%1)").arg(program)};
  return QProcess::startDetached(runas, cmds);  // 使用startDetached来异步启动进程
}

bool MountHelper::IsAdministrator() {
  BOOL isAdmin = FALSE;
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  PSID AdministratorsGroup;
  if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
    if (!CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin)) {
      isAdmin = FALSE;
    }
    FreeSid(AdministratorsGroup);
  }
  return isAdmin == TRUE;
}

// 修改后的挂载函数
bool MountHelper::MountVolume(const QString& volumeGuid, const QString& label, QString& volMountPoint) {
  if (volumeGuid.isEmpty() || label.isEmpty()) {
    qWarning("volumeGuid[%s] and label[%s] cannot empty", qPrintable(volumeGuid), qPrintable(label));
    return false;
  }
  if (!isVolumeAvailable(volumeGuid)) {
    qWarning("volumeGuid[%s] not exist at all, skip mount", qPrintable(volumeGuid));
    return false;
  }
  const QString& MOUNT_POINT_ROOT{"C:/mnt/" + label};
  if (!QFileInfo{MOUNT_POINT_ROOT}.isDir()) {
    qDebug("path[%s] not exist, create now", qPrintable(MOUNT_POINT_ROOT));
    if (!QDir{}.mkpath(MOUNT_POINT_ROOT)) {
      qWarning("path[%s] create failed", qPrintable(MOUNT_POINT_ROOT));
      return false;
    }
  }
  //  "\\?\Volume{GUID}\"
  volMountPoint = QDir::toNativeSeparators(MOUNT_POINT_ROOT) + R"(\)";
  const QString volumeName{VOLUME_NAME_TEMPLATE.arg(volumeGuid)};
  if (!SetVolumeMountPointW(volMountPoint.toStdWString().c_str(), volumeName.toStdWString().c_str())) {
    // #define ERROR_ACCESS_DENIED __MSABI_LONG(5)
    // #define ERROR_INVALID_NAME __MSABI_LONG(123)
    // #define ERROR_DIR_NOT_EMPTY __MSABI_LONG(145)
    qWarning("volume Name[%s] mount on[%s], resultCode:%lu", qPrintable(volumeName), qPrintable(volMountPoint), GetLastError());
    return false;
  }
  qDebug("volume Name[%s] mount on[%s] succeed", qPrintable(volumeName), qPrintable(volMountPoint));
  return true;
}

bool MountHelper::UnmountVolume(const QString& volMountPoint) {
  return DeleteVolumeMountPointA(volMountPoint.toStdString().c_str()) == true;
}

// 枚举指定卷的所有挂载点
QSet<QString> MountHelper::GetMountPointsByVolumeName(const wchar_t* volumeName) {
  WCHAR mountPoint[MAX_PATH] = {0};
  HANDLE hFind = FindFirstVolumeMountPointW(volumeName, mountPoint, MAX_PATH);
  if (hFind == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_NO_MORE_ITEMS) {
      qDebug("no %ls find at all", volumeName);
    }
    return {};
  }
  QSet<QString> pnts;
  do {
    pnts << QString::fromWCharArray(mountPoint);
  } while (FindNextVolumeMountPointW(hFind, mountPoint, MAX_PATH));

  FindVolumeMountPointClose(hFind);
  return pnts;
}

// 主调用逻辑
QMap<QString, QSet<QString> > MountHelper::Volumes2ContainedMountPnts() {
  WCHAR volumeName[MAX_PATH] = {0};
  HANDLE hVol = FindFirstVolumeW(volumeName, ARRAYSIZE(volumeName));
  QMap<QString, QSet<QString> > vol2Pnts;

  do {
    // 过滤系统保留卷
    if (wcsstr(volumeName, L"\\\\?\\Volume") == nullptr) {
      continue;
    }
    const QSet<QString>& pnts = GetMountPointsByVolumeName(volumeName);
    size_t len = wcslen(volumeName);
    // 转换卷名格式：去除末尾反斜杠
    if (len > 0 && volumeName[len - 1] == L'\\') {
      volumeName[len - 1] = L'\0';
    }
    vol2Pnts[QString::fromWCharArray(volumeName)] = pnts;
  } while (FindNextVolumeW(hVol, volumeName, ARRAYSIZE(volumeName)));

  FindVolumeClose(hVol);
  return vol2Pnts;
}

QMap<QString, QString> MountHelper::GetGuid2LabelMap() {
  wchar_t volumeName[MAX_PATH]{0};
  HANDLE hFind = FindFirstVolumeW(volumeName, MAX_PATH);
  if (hFind == INVALID_HANDLE_VALUE) {
    qWarning("find first volume failed.");
    return {};
  }

  QMap<QString, QString> guid2Label;
  wchar_t labelBuf[MAX_PATH]{0};
  do {
    memset(labelBuf, 0, sizeof(labelBuf));
    if (GetVolumeInformationW(volumeName, labelBuf, MAX_PATH, NULL, NULL, NULL, NULL, 0)) {
      const QString label{QString::fromWCharArray(labelBuf)};
      const QString guid{ExtractGuidFromVolumeName(volumeName)};  // 提取GUID
      guid2Label[guid] = label;
    }
  } while (FindNextVolumeW(hFind, volumeName, MAX_PATH));
  qDebug("%d guid find succeed", guid2Label.size());
  return guid2Label;
}

bool MountHelper::GetVolumeInfo(const QString& path, QString& volName) {
  TCHAR volumeName[MAX_PATH] = {0};
  TCHAR fileSystemName[MAX_PATH] = {0};
  DWORD serialNumber[3]{0};  // serialNumber 128位
  DWORD maxComponentLength = 0;
  DWORD fileSystemFlags = 0;

  if (!GetVolumeInformation(path.toStdWString().c_str(),                          //
                            volumeName, MAX_PATH,                                 //
                            serialNumber, &maxComponentLength, &fileSystemFlags,  //
                            fileSystemName, MAX_PATH)) {
    qWarning("Failed to retrieve GUID by path[%s] error:%lu", qPrintable(path), GetLastError());
    return false;
  }
  volName = QString::fromWCharArray(volumeName);
  return true;
}

bool MountHelper::GetGuidByDrive(const QString& driveStr, QString& guid) {
  const QString& drvPath = QDir::toNativeSeparators(driveStr);
  TCHAR volumeName[MAX_PATH] = {0};
  if (!GetVolumeNameForVolumeMountPoint(drvPath.toStdWString().c_str(), volumeName, MAX_PATH)) {
    return false;
  }
  guid = ExtractGuidFromVolumeName(volumeName);
  return true;
}

QString MountHelper::FindRootByGUIDWin(const QString& targetGuid) {
  WCHAR volumePathNames[MAX_PATH] = {0};
  DWORD bufferSize = MAX_PATH;
  QString rootPath;

  // 转换为Windows卷名格式
  const QString& volumeName = VOLUME_NAME_TEMPLATE.arg(targetGuid);

  // 调用API获取挂载点列表
  if (GetVolumePathNamesForVolumeNameW(volumeName.toStdWString().c_str(), volumePathNames, MAX_PATH, &bufferSize)) {
    // 解析多字符串结构（以双空字符结尾）
    WCHAR* p = volumePathNames;
    while (*p != '\0') {
      QString path = QString::fromWCharArray(p);
      // 筛选盘符型根目录（如C:\）
      if (path.length() == 3 && path[1] == L':') {
        rootPath = QDir::toNativeSeparators(path);
        break;
      }
      p += wcslen(p) + 1;
    }
  }
  return rootPath;
}

const MountHelper::GUID_2_PNTS_SET& MountHelper::Guids2MntPntSet(bool forceRefresh) {
  static GUID_2_PNTS_SET guid2MntPnts;
  if (forceRefresh || guid2MntPnts.isEmpty()) {
    const QList<QStorageInfo>& silst = QStorageInfo::mountedVolumes();
    for (const auto& si : silst) {
      const QString& volumeFull = QString::fromUtf8(si.device());
      const QString& volume = ExtractGuidFromVolumeName(volumeFull);
      const QString& rp = QDir::toNativeSeparators(si.rootPath());  // L"C:\\"
      guid2MntPnts[volume] = GetMountPointsByVolumeName(rp.toStdWString().c_str());
    }
  }
  return guid2MntPnts;
}

QString MountHelper::GetDisplayNameByGuidTableName(QString guidTableName) {
  if (guidTableName.size() != MountHelper::GUID_LEN) {
    return guidTableName;
  }
  const QString& stdVolumeName = VOLUME_NAME_TEMPLATE.arg(guidTableName.replace(TABLE_UNDERSCORE, GUID_HYPEN));
  const auto& mountedVolLst = QStorageInfo::mountedVolumes();
  for (const auto& si : mountedVolLst) {
    const QString& volumeFull = QString::fromUtf8(si.device());
    if (volumeFull == stdVolumeName) {
      return si.displayName();
    }
  }
  return QString("Display name of[%1] not found").arg(stdVolumeName);
}

// GUID in underscore -> RootPath
QMap<QString, QString> MountHelper::GetGuidTableName2DisplayName() {
  QMap<QString, QString> guidTblName2Disp;
  const auto& mountedVolLst = QStorageInfo::mountedVolumes();
  for (const auto& si : mountedVolLst) {
    const QString& volumeFull = QString::fromUtf8(si.device());
    QString volume = ExtractGuidFromVolumeName(volumeFull);
    guidTblName2Disp[volume.replace(GUID_HYPEN, TABLE_UNDERSCORE)] = si.displayName();
  }
  return guidTblName2Disp;
}

QStringList MountHelper::GetGuidJoinDisplayName() {
  QStringList guidDispLst;
  const auto& mountedVolLst = QStorageInfo::mountedVolumes();
  for (const auto& si : mountedVolLst) {
    const QString& volumeFull = QString::fromUtf8(si.device());
    QString volumeAlsoTableName = ExtractGuidFromVolumeName(volumeFull);
    volumeAlsoTableName.replace(GUID_HYPEN, TABLE_UNDERSCORE);
    guidDispLst << volumeAlsoTableName + JOINER_STR + si.displayName();
  }
  return guidDispLst;
}

QString MountHelper::ChoppedDisplayName(const QString& guidJoinDisplayName) {
  const int colonIndex = guidJoinDisplayName.indexOf(JOINER_STR);
  if (colonIndex == -1) {
    return guidJoinDisplayName;
  }
  return guidJoinDisplayName.left(colonIndex);
}

#endif
