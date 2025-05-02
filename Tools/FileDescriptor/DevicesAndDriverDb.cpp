#include "DevicesAndDriverDb.h"
#include "MountHelper.h"
#include <QStorageInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSet>
// Driver, Volume Label, Root Path, Avail, Total, GUID, MOUNTED_PATH
// mount
// umount

// 定义一个辅助宏来简化枚举到字符串的映射
#define CASE_ENUM_TO_STRING(enum_val) \
  case enum_val:                      \
    return #enum_val

const char* DEV_DRV_TABLE::GetFILEDString(FIELD_E enum_val) {
  switch (enum_val) {
    CASE_ENUM_TO_STRING(ROOT_PATH);
    CASE_ENUM_TO_STRING(VOLUME_LABEL);
    CASE_ENUM_TO_STRING(TOTAL_BYTES);
    CASE_ENUM_TO_STRING(AVAIL_BYTES);
    CASE_ENUM_TO_STRING(GUID);
    CASE_ENUM_TO_STRING(MOUNT_POINT);
    CASE_ENUM_TO_STRING(ADT_TIME);
    CASE_ENUM_TO_STRING(FILED_BUTT);
    default: {
      qWarning("enum_val[%d] out of range", enum_val);
      static const char UNKNOWN[]{"Unknown"};
      return UNKNOWN;
    }
  }
}

using namespace DEV_DRV_TABLE;

const QString DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE  //
    {"CREATE TABLE IF NOT EXISTS `%1` ("                   //
     + QString("   `%1` NCHAR(260) NOT NULL,"              // ROOT_PATH
               "   `%2` NCHAR(260) DEFAULT \"\","          // VOLUME_LABEL
               "   `%3` INTEGER DEFAULT 0,"                // TOTAL_BYTES
               "   `%4` INTEGER DEFAULT 0,"                // AVAIL_BYTES
               "   `%5` CHAR(36) NOT NULL,"                // GUID, GUID_LEN = 36
               "   `%6` NCHAR(260) DEFAULT \"\","          // MOUNTED_PATH
               "   `%7` INTEGER DEFAULT 0,"                // ADT_TIME
               "    PRIMARY KEY (%5)"
               "    );")
           .arg(GetFILEDString(ROOT_PATH))     //
           .arg(GetFILEDString(VOLUME_LABEL))  //
           .arg(GetFILEDString(TOTAL_BYTES))   //
           .arg(GetFILEDString(AVAIL_BYTES))   //
           .arg(GetFILEDString(GUID))          //
           .arg(GetFILEDString(MOUNT_POINT))   //
           .arg(GetFILEDString(ADT_TIME))};    //

const QString DevicesAndDriverDb::INSERT_DEV_DRV_TEMPLATE  //
    {"REPLACE INTO `%1` "                                  //
     + QString(R"(
(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7`)
VALUES(:%1, :%2, :%3, :%4, :%5, :%6, :%7);)")
           .arg(GetFILEDString(ROOT_PATH))     //
           .arg(GetFILEDString(VOLUME_LABEL))  //
           .arg(GetFILEDString(TOTAL_BYTES))   //
           .arg(GetFILEDString(AVAIL_BYTES))   //
           .arg(GetFILEDString(GUID))          //
           .arg(GetFILEDString(MOUNT_POINT))   //
           .arg(GetFILEDString(ADT_TIME))};    //

const QString DevicesAndDriverDb::UPDATE_SIZE_TEMPLATE  //
    {"UPDATE `%1` "                                     //
     + QString(R"(
SET `%1` = :%1, `%2` = :%2, `%3` = :%3, `%4` = :%4
WHERE `%5` = :%5;)")
           .arg(GetFILEDString(ROOT_PATH))     //
           .arg(GetFILEDString(VOLUME_LABEL))  //
           .arg(GetFILEDString(TOTAL_BYTES))   //
           .arg(GetFILEDString(AVAIL_BYTES))   //
           .arg(GetFILEDString(GUID))};

const QString DevicesAndDriverDb::UPDATE_ADT_TIME_TEMPLATE  //
    {"UPDATE %1 "                                           //
     + QString{"SET `%1` = :%1 WHERE `%2` = :%2;"}          //
           .arg(GetFILEDString(ADT_TIME))                   //
           .arg(GetFILEDString(GUID))};

const QString DevicesAndDriverDb::UPDATE_MOUNT_POINT_TEMPLATE  //
    {"UPDATE %1 "                                              //
     + QString{"SET `%1` = :%1 WHERE `%2` = :%2;"}             //
           .arg(GetFILEDString(MOUNT_POINT))                   //
           .arg(GetFILEDString(GUID))};

QList<VolumeInfo> GetVolumesInfo() {
  QList<VolumeInfo> ans;
  const auto& mountedVolLst = QStorageInfo::mountedVolumes();
  for (const auto& storageInfo : mountedVolLst) {
    const QString& drvPath{storageInfo.rootPath()};
    QString guid;
    if (!MountHelper::GetGuidByDrive(drvPath, guid)) {
      qWarning("Get guid of driver[%s] failed", qPrintable(drvPath));
      return {};
    }
    ans << VolumeInfo{drvPath, storageInfo.name(), storageInfo.bytesTotal(), storageInfo.bytesAvailable(), guid};
  }
  return ans;
}

int DevicesAndDriverDb::InitDeviceAndDriver(const QString& tableName) {
  if (tableName.isEmpty()) {
    qDebug("tableName[%s] invalid", qPrintable(tableName));
    return FD_TABLE_NAME_INVALID;
  }

  if (!IsTableEmpty(tableName)) {
    qDebug("table[%s] not empty, skip init", qPrintable(tableName));
    return FD_OK;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  QSqlQuery query{db};
  if (!query.prepare(INSERT_DEV_DRV_TEMPLATE.arg(tableName))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  const auto& vols = GetVolumesInfo();
  for (const auto& vol : vols) {
    // no known conversion from QString to QVariant. Causes: header QVariant not include
    query.bindValue(ROOT_PATH, vol.rootPath);
    query.bindValue(VOLUME_LABEL, vol.volumeLabel);
    query.bindValue(TOTAL_BYTES, vol.totalBytes);
    query.bindValue(AVAIL_BYTES, vol.availBytes);
    query.bindValue(GUID, vol.guid);
    if (!query.exec()) {
      db.rollback();
      qWarning("Insert[%s] failed: %s",  //
               qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_INVALID;
    }
  }

  query.finish();
  qWarning("%d record(s) commit insert into succeed", vols.size());
  return vols.size();
}

int DevicesAndDriverDb::UpdateDeviceAndDriver(const QString& tableName, VolumeUpdateResult* pRst, FUNC_VOLUME_INFO_GETTER pGetter) {
  if (pGetter == nullptr) {
    qWarning("pGetter is nullptr");
    return FD_INVALID;
  }

  if (tableName.isEmpty()) {
    return FD_TABLE_NAME_INVALID;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  QSet<QString> existedGuids, newGuids;
  if (!QueryPK(tableName, VOLUME_ENUM_TO_STRING(GUID), existedGuids)) {
    qWarning("no guid(s) find at all");
  }
  const auto& volumeInfos = pGetter();
  for (const auto& volumeInfo : volumeInfos) {
    newGuids.insert(volumeInfo.guid);
  }
  auto needInsertGuids{newGuids};
  needInsertGuids.subtract(existedGuids);
  auto needDeleteGuids{existedGuids};
  needDeleteGuids.subtract(newGuids);
  auto needUpdateGuids{newGuids};
  needUpdateGuids.intersect(existedGuids);
  qDebug("guids insert:%d, delete:%d, update:%d", needInsertGuids.size(), needDeleteGuids.size(), needUpdateGuids.size());

  // before insert check if at least 1 guid
  if (!needInsertGuids.isEmpty()) {
    QSqlQuery query{db};
    if (!query.prepare(INSERT_DEV_DRV_TEMPLATE.arg(tableName))) {
      qWarning("prepare command[%s] failed: %s",  //
               qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_PREPARE_FAILED;
    }

    if (!db.transaction()) {
      qWarning("start the %dth transaction failed: %s",  //
               1, qPrintable(db.lastError().text()));
      return FD_TRANSACTION_FAILED;
    }

    for (const auto& volumeInfo : volumeInfos) {
      if (!needInsertGuids.contains(volumeInfo.guid)) {
        continue;
      }
      query.bindValue(ROOT_PATH, volumeInfo.rootPath);  // no known conversion from QString to QVariant. Causes: header QVariant not include
      query.bindValue(VOLUME_LABEL, volumeInfo.volumeLabel);
      query.bindValue(TOTAL_BYTES, volumeInfo.totalBytes);
      query.bindValue(AVAIL_BYTES, volumeInfo.availBytes);
      query.bindValue(GUID, volumeInfo.guid);
      if (!query.exec()) {
        db.rollback();
        qWarning("replace[%s] failed: %s",  //
                 qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
        return FD_INVALID;
      }
    }
    if (!db.commit()) {
      db.rollback();
      qWarning("remain record(s) commit failed: %s", qPrintable(db.lastError().text()));
      return FD_COMMIT_FAILED;
    }
    query.finish();
    qDebug("%d record(s) to be insert...", needInsertGuids.size());
  }

  // before delete check if at least 1 guid
  if (!needDeleteGuids.isEmpty()) {
    QString qryCmd = QString(R"(DELETE FROM %1 WHERE `%2` IN (")").arg(tableName, VOLUME_ENUM_TO_STRING(GUID));
    const QStringList guids{needDeleteGuids.cbegin(), needDeleteGuids.cend()};
    qryCmd += guids.join(R"(",")");
    qryCmd += R"("))";
    QSqlQuery query{db};
    if (!query.exec(qryCmd)) {
      db.rollback();
      qWarning("delete[%s] failed: %s",  //
               qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_INVALID;
    }
    query.finish();
    qDebug("%d record(s) to be deleted...", needDeleteGuids.size());
  }

  // before update check if at least 1 guid
  if (!needUpdateGuids.isEmpty()) {
    QSqlQuery query{db};
    if (!query.prepare(UPDATE_SIZE_TEMPLATE.arg(tableName))) {
      qWarning("prepare command[%s] failed: %s",  //
               qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return FD_PREPARE_FAILED;
    }
    // 开始事务
    if (!db.transaction()) {
      qWarning("start the %dth transaction failed: %s",  //
               1, qPrintable(db.lastError().text()));
      return FD_TRANSACTION_FAILED;
    }
    query.prepare(UPDATE_SIZE_TEMPLATE.arg(tableName));
    for (const auto& volumeInfo : volumeInfos) {
      if (!needUpdateGuids.contains(volumeInfo.guid)) {
        continue;
      }
      query.bindValue(ROOT_PATH, volumeInfo.rootPath);
      query.bindValue(VOLUME_LABEL, volumeInfo.volumeLabel);
      query.bindValue(TOTAL_BYTES, volumeInfo.totalBytes);
      query.bindValue(AVAIL_BYTES, volumeInfo.availBytes);
      query.bindValue(GUID, volumeInfo.guid);
      if (!query.exec()) {
        db.rollback();
        qWarning("update[%s] failed: %s",  //
                 qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
        return FD_INVALID;
      }
    }
    if (!db.commit()) {
      db.rollback();
      qWarning("remain update record(s) commit failed: %s", qPrintable(db.lastError().text()));
      return FD_COMMIT_FAILED;
    }
    query.finish();
    qDebug("%d record(s) to be update...", needUpdateGuids.size());
  }

  if (pRst != nullptr) {
    pRst->deleteCnt = needDeleteGuids.size();
    pRst->insertCnt = needInsertGuids.size();
    pRst->updateCnt = needUpdateGuids.size();
  }

  qDebug("insert:%d, delete:%d, update:%d record(s) succeed", needInsertGuids.size(), needDeleteGuids.size(), needUpdateGuids.size());
  return FD_OK;
}

int DevicesAndDriverDb::UpdateAdtTime(const QString& tableName, const QString& guid, int adtTime) {
  if (tableName.isEmpty()) {
    return FD_TABLE_NAME_INVALID;
  }

  if (guid.isEmpty()) {
    return FD_FIELD_VALUE_INVALID;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  QSqlQuery query(db);
  if (!query.prepare(UPDATE_ADT_TIME_TEMPLATE.arg(tableName))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }
  // bindValue(int depend on `?` or `:XXX` sequence
  query.bindValue(0, adtTime);
  query.bindValue(1, guid);
  if (!query.exec()) {
    db.rollback();
    qWarning("replace[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_INVALID;
  }
  const QString& qryStr = query.lastQuery();

  query.finish();
  qWarning("replaced adtTime:%d where guid=%s succeed", adtTime, qPrintable(guid));
  return FD_OK;
}

// 外部修改数据库后还需强制刷新模型, 显示内容才会生效
// model->select();
int DevicesAndDriverDb::UpdateMountedPath(const QString& tableName, const QString& guid, const QString& mountedPath) {
  if (tableName.isEmpty()) {
    return FD_TABLE_NAME_INVALID;
  }

  if (guid.isEmpty()) {
    return FD_FIELD_VALUE_INVALID;
  }

  auto db = GetDb();
  if (!CheckValidAndOpen(db)) {
    return FD_DB_OPEN_FAILED;
  }

  QSqlQuery query(db);
  if (!query.prepare(UPDATE_MOUNT_POINT_TEMPLATE.arg(tableName))) {
    qWarning("prepare command[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_PREPARE_FAILED;
  }

  query.bindValue(0, mountedPath);
  query.bindValue(1, guid);

  if (!query.exec()) {
    db.rollback();
    qWarning("update[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_INVALID;
  }

  query.finish();
  qWarning("replaced mountedPath:%s where guid=%s succeed", qPrintable(mountedPath), qPrintable(guid));
  return FD_OK;
}
