#ifndef DEVICESANDDRIVERDB_H
#define DEVICESANDDRIVERDB_H

#include "DbManager.h"

struct VolumeInfo {
  QString rootPath;
  QString volumeLabel;
  qint64 totalBytes;
  qint64 availBytes;
  QString guid;
};

QList<VolumeInfo> GetVolumesInfo();
using FUNC_VOLUME_INFO_GETTER = decltype(GetVolumesInfo)*;

class DevicesAndDriverDb : public DbManager {
 public:
  DevicesAndDriverDb(const QString& dbName, const QString& connName, QObject* parent = nullptr)  //
      : DbManager{dbName, connName, parent} {                                                    //
  }
  int InitDeviceAndDriver(const QString& tableName);
  FD_ERROR_CODE AdtDeviceAndDriver(const QString& tableName, VolumeUpdateResult* pRst = nullptr, FUNC_VOLUME_INFO_GETTER pGetter = GetVolumesInfo);
  int UpdateAdtTime(const QString& tableName, const QString& guid, int adtTime);
  int UpdateMountedPath(const QString& tableName, const QString& guid, const QString& mountedPath);
  static const QString CREATE_DEV_DRV_TEMPLATE;
  static const QString INSERT_DEV_DRV_FULL_TEMPLATE;
  static const QString INSERT_DEV_DRV_TEMPLATE;
  static const QString UPDATE_SIZE_TEMPLATE;
  static const QString UPDATE_ADT_TIME_TEMPLATE;
  static const QString UPDATE_MOUNT_POINT_TEMPLATE;
 private:
  FD_ERROR_CODE Insert(const QString& tableName, QSet<QString> needInsertGuids, const QList<VolumeInfo>& volumeInfos, int& insertCnt);
  FD_ERROR_CODE Delete(const QString& tableName, const QSet<QString>& needDeleteGuids, int& deleteCnt);
  FD_ERROR_CODE Update(const QString& tableName, const QSet<QString>& needUpdateGuids, const QList<VolumeInfo>& volumeInfos, int& updateCnt);
};

#endif  // DEVICESANDDRIVERDB_H
