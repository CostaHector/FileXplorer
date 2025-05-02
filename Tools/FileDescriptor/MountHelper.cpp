#include "MountHelper.h"
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include <QProcess>

#ifndef WIN32
QString findVolumeGuidByLabel(const QString& label) {
  return "";
}
bool isVolumeAvailable(const QString& volumeGuid) {
  return false;
}
bool mountVolumeToGuidPath(const QString& volumeGuid) {
  return false;
}
QString resolveFilePath(const QString& relativePath, const QString& volumeGuid) {
  return "";
}
QMap<QString, QString> GetCurrentGuid2Label() {
  return {};
}
#else

#include <windows.h>
#include "fileapi.h"

QString ExtractGuidFromVolumeName(const wchar_t (&volumeName)[MAX_PATH]) {
  // R"(\\?\Volume{36 chars}\)"
  static const char VOLUME_PREFIX[] = R"(\\?\Volume{)";
  static constexpr int START_INT = sizeof(VOLUME_PREFIX) - 1;
  static constexpr int GUID_LEN = 36;
  return QString::fromWCharArray(volumeName).mid(START_INT, GUID_LEN);
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
bool MountHelper::isVolumeAvailable(const QString& volumeGuid) {
  wchar_t volumeName[MAX_PATH]{0};
  HANDLE hFind = FindFirstVolumeW(volumeName, MAX_PATH);
  if (hFind == INVALID_HANDLE_VALUE) {
    qWarning("find first volume failed. volumeGuid[%s] not exist", qPrintable(volumeGuid));
    return false;
  }

  do {
    const QString currentGuid{ExtractGuidFromVolumeName(volumeName)};  // 提取GUID部分
    if (currentGuid == volumeGuid) {
      FindVolumeClose(hFind);
      return true;
    }
  } while (FindNextVolumeW(hFind, volumeName, MAX_PATH));

  FindVolumeClose(hFind);
  return false;
}

bool MountHelper::RunAsAdmin() {
  const QString& program = "path\\to\\yourapp.exe"; // 替换为你的应用程序路径
  QString runas{"runas"};
  QStringList cmds{"/user:administrator", QString(R"(%1)").arg(program)};
  return QProcess::startDetached(runas, cmds); // 使用startDetached来异步启动进程
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
  const QString volumeName{QString{R"(\\?\Volume{%1}\)"}.arg(volumeGuid)};
  if (!SetVolumeMountPointW(volMountPoint.toStdWString().c_str(), volumeName.toStdWString().c_str())) {
    // #define ERROR_ACCESS_DENIED __MSABI_LONG(5)
    // #define ERROR_INVALID_NAME __MSABI_LONG(123)
    // #define ERROR_DIR_NOT_EMPTY __MSABI_LONG(145)
    qWarning("volume Name[%s] mount on[%s], resultCode:%d", qPrintable(volumeName), qPrintable(volMountPoint), GetLastError());
    return false;
  }
  qDebug("volume Name[%s] mount on[%s] succeed", qPrintable(volumeName), qPrintable(volMountPoint));
  return true;
}

bool MountHelper::UnmountVolume(const QString& volMountPoint) {
  return DeleteVolumeMountPointA(volMountPoint.toStdString().c_str()) == true;
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
    qWarning("Failed to retrieve GUID by path[%s] error:%u", qPrintable(path), GetLastError());
    return false;
  }
  volName = QString::fromWCharArray(volumeName);
  return true;
}

WINBASEAPI WINBOOL WINAPI GetVolumeNameForVolumeMountPointW(LPCWSTR lpszVolumeMountPoint, LPWSTR lpszVolumeName, DWORD cchBufferLength);

bool MountHelper::GetGuidByDrive(const QString& driveStr, QString& guid) {
  const QString& drvPath = QDir::toNativeSeparators(driveStr);
  TCHAR volumeName[MAX_PATH] = {0};
  if (!GetVolumeNameForVolumeMountPoint(drvPath.toStdWString().c_str(), volumeName, MAX_PATH)) {
    return false;
  }
  guid = ExtractGuidFromVolumeName(volumeName);
  return true;
}
#endif

