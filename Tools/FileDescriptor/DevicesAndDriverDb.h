#ifndef DEVICESANDDRIVERDB_H
#define DEVICESANDDRIVERDB_H

#include "DbManager.h"

namespace DEV_DRV_TABLE {
enum FIELD_E {
  ROOT_PATH = 0,  //
  VOLUME_LABEL,   //
  TOTAL_BYTES,    //
  AVAIL_BYTES,    //
  GUID,           //
  MOUNT_POINT,    //
  ADT_TIME,       //
  FILED_BUTT
};

#define VOLUME_ENUM_TO_STRING(enum_val) #enum_val
#define VOLUME_ENUM_TO_COLON_STRING(enum_val) ":"#enum_val

// 使用一个函数来返回枚举项的字符串表示
const char* GetFILEDString(FIELD_E enum_val);
};  // namespace DEV_DRV_TABLE


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
  int UpdateDeviceAndDriver(const QString& tableName, FUNC_VOLUME_INFO_GETTER pGetter = GetVolumesInfo, VolumeUpdateResult* pRst = nullptr);
  int UpdateAdtTime(const QString& tableName, const QString& guid, int adtTime);
  int UpdateMountedPath(const QString& tableName, const QString& guid, const QString& mountedPath);

  static const QString CREATE_DEV_DRV_TEMPLATE;
  static const QString INSERT_DEV_DRV_TEMPLATE;
  static const QString UPDATE_SIZE_TEMPLATE;
  static const QString UPDATE_ADT_TIME_TEMPLATE;
  static const QString UPDATE_MOUNT_POINT_TEMPLATE;
};

#endif  // DEVICESANDDRIVERDB_H
