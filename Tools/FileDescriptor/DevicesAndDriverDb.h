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

struct VolumeUpdateResult {
  void Init() {
    deleteCnt = 0;
    insertCnt = 0;
    updateCnt = 0;
  }
  int deleteCnt;
  int insertCnt;
  int updateCnt;
};

QList<VolumeInfo> GetVolumesInfo();
using FUNC_VOLUME_INFO_GETTER = decltype(GetVolumesInfo)*;

class DevicesAndDriverDb : public DbManager {
 public:
  DevicesAndDriverDb(const QString& dbName, const QString& connName, QObject* parent = nullptr)  //
      : DbManager{dbName, connName, parent} {                                                    //
  }
  int InitDeviceAndDriver(const QString& tableName);
  int UpdateDeviceAndDriver(const QString& tableName, VolumeUpdateResult* pRst = nullptr, FUNC_VOLUME_INFO_GETTER pGetter = GetVolumesInfo);
  int UpdateAdtTime(const QString& tableName, const QString& guid, int adtTime);
  int UpdateMountedPath(const QString& tableName, const QString& guid, const QString& mountedPath);
  static const QString CREATE_DEV_DRV_TEMPLATE;
  static const QString INSERT_DEV_DRV_TEMPLATE;
  static const QString UPDATE_SIZE_TEMPLATE;
  static const QString UPDATE_ADT_TIME_TEMPLATE;
  static const QString UPDATE_MOUNT_POINT_TEMPLATE;
};

#endif  // DEVICESANDDRIVERDB_H
