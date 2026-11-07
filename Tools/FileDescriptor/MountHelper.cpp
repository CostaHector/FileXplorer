#include "MountHelper.h"
#include "TableNameHelper.h"

#include <QDir>
#include <QStorageInfo>
#include <QRegularExpression>

namespace MountPathTableNameMapper {
bool isMountPointOnline(const QString& mountPoint) {
  const QStorageInfo storage(mountPoint);
  // 路径对应一个有效的挂载点 && 文件系统已准备好进行读写
  return storage.isValid() && storage.isReady();
}

QString toTableName(const QString& mountPath) {
  static const QRegularExpression validMountPath{"^(C:|/mnt)/DISKS/[a-zA-Z0-9]+$"};
  if (!validMountPath.match(mountPath).hasMatch()) {
    QString tableNameDecode = TableNameHelper::WorkPath2TableName(mountPath);
    return tableNameDecode;
  }
  QString tableName = mountPath;
  return tableName.replace('/', '_').replace(':', '_');
}

QString toMountPath(const QString& tableName, bool& bConversionOk) {
  static const QRegularExpression validTableName{"^(C_|_mnt)_DISKS_[a-zA-Z0-9]+$"};
  if (!validTableName.match(tableName).hasMatch()) {
    QString pathCode = TableNameHelper::TableName2WorkPath(tableName, bConversionOk);
    return bConversionOk ? pathCode : "";
  }
  bConversionOk = true;
  QString mountPath = tableName;
  if (mountPath.size() >= 3 && mountPath[0] == 'C' && mountPath[1] == '_' && mountPath[2] == '_') {
    mountPath[1] = ':';
  }
  mountPath.replace('_', '/');
  return mountPath;
}

QStringList CandidateTableNamesList() {
  QString mountPointRoot;
#ifdef _WIN32
  mountPointRoot = "C:/DISKS";
#else
  mountPointRoot = "/mnt/DISKS";
#endif

  const QDir mountPntRootDir(mountPointRoot, "", QDir::SortFlag::Name, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden);
  const QStringList mountPntFolderCandidates{mountPntRootDir.entryList()};

  QStringList tableNamesCandidates;
  tableNamesCandidates.reserve(mountPntFolderCandidates.size());
  for (const QString& mountPntFolder : mountPntFolderCandidates) {
    const QString mountPntFullPath = mountPntRootDir.absoluteFilePath(mountPntFolder);
    QString validTableName = toTableName(mountPntFullPath);
    tableNamesCandidates.push_back(validTableName);
  }
  return tableNamesCandidates;
}

}  // namespace MountPathTableNameMapper
