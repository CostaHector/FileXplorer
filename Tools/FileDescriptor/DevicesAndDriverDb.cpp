#include "DevicesAndDriverDb.h"
#include "MountHelper.h"
#include "TableFields.h"
#include "public/PublicMacro.h"
#include <QStorageInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSet>
using namespace DEV_DRV_TABLE;

const QString DevicesAndDriverDb::CREATE_DEV_DRV_TEMPLATE  //
    {"CREATE TABLE IF NOT EXISTS `%1` ("                   //
     + QString("   `%1` NCHAR(260) NOT NULL,"              // ROOT_PATH
               "   `%2` NCHAR(260) DEFAULT \"\","          // VOLUME_LABEL
               "   `%3` INTEGER DEFAULT 0,"                // TOTAL_BYTES
               "   `%4` INTEGER DEFAULT 0,"                // AVAIL_BYTES
               "   `%5` CHAR(36) NOT NULL,"                // GUID, GUID_LEN = 36
               "   `%6` NCHAR(512) DEFAULT \"\","          // MOUNTED_PATH
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

const QString DevicesAndDriverDb::INSERT_DEV_DRV_FULL_TEMPLATE  //
    {
        "REPLACE INTO `%1` "  //
        + QString(R"(
(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`, `%7`)
VALUES(:%1, :%2, :%3, :%4, :%5, :%6, :%7);)")
              .arg(GetFILEDString(ROOT_PATH))     //
              .arg(GetFILEDString(VOLUME_LABEL))  //
              .arg(GetFILEDString(TOTAL_BYTES))   //
              .arg(GetFILEDString(AVAIL_BYTES))   //
              .arg(GetFILEDString(GUID))          //
              .arg(GetFILEDString(MOUNT_POINT))   //
              .arg(GetFILEDString(ADT_TIME))      //
    };                                            //

const QString DevicesAndDriverDb::INSERT_DEV_DRV_TEMPLATE  //
    {
        "REPLACE INTO `%1` "  //
        + QString(R"(
(`%1`, `%2`, `%3`, `%4`, `%5`, `%6`)
VALUES(:%1, :%2, :%3, :%4, :%5, :%6);)")
              .arg(GetFILEDString(ROOT_PATH))     //
              .arg(GetFILEDString(VOLUME_LABEL))  //
              .arg(GetFILEDString(TOTAL_BYTES))   //
              .arg(GetFILEDString(AVAIL_BYTES))   //
              .arg(GetFILEDString(GUID))          //
              .arg(GetFILEDString(MOUNT_POINT))   //
    };                                            //

const QString DevicesAndDriverDb::UPDATE_SIZE_TEMPLATE  //
    {
        "UPDATE `%1` "  //
        + QString(R"(
SET `%1` = :%1, `%2` = :%2, `%3` = :%3, `%4` = :%4, `%5` = :%5
WHERE `%6` = :%6;)")
              .arg(GetFILEDString(ROOT_PATH))     //
              .arg(GetFILEDString(VOLUME_LABEL))  //
              .arg(GetFILEDString(TOTAL_BYTES))   //
              .arg(GetFILEDString(AVAIL_BYTES))   //
              .arg(GetFILEDString(MOUNT_POINT))   //
              .arg(GetFILEDString(GUID))          //
    };

enum UPDATE_SIZE_FIELD {
  UPDATE_SIZE_FIELD_ROOT_PATH = 0,
  UPDATE_SIZE_FIELD_VOLUME_LABEL,
  UPDATE_SIZE_FIELD_TOTAL_BYTES,
  UPDATE_SIZE_FIELD_AVAIL_BYTES,
  UPDATE_SIZE_FIELD_MOUNT_POINT,
  UPDATE_SIZE_FIELD_GUID
};

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

  const auto& guid2Pnts = MountHelper::Guids2MntPntSet(true);
  const auto& vols = GetVolumesInfo();
  for (const auto& vol : vols) {
    // no known conversion from QString to QVariant. Causes: header QVariant not include
    const QSet<QString>& pntsSet = guid2Pnts.value(vol.guid, {});
    const QStringList pntsLst{pntsSet.cbegin(), pntsSet.cend()};
    query.bindValue(ROOT_PATH, vol.rootPath);
    query.bindValue(VOLUME_LABEL, vol.volumeLabel);
    query.bindValue(TOTAL_BYTES, vol.totalBytes);
    query.bindValue(AVAIL_BYTES, vol.availBytes);
    query.bindValue(GUID, vol.guid);
    query.bindValue(MOUNT_POINT, pntsLst.join('\n'));
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

FD_ERROR_CODE DevicesAndDriverDb::Insert(const QString& tableName, QSet<QString> needInsertGuids, const QList<VolumeInfo>& volumeInfos, int& insertCnt) {
  insertCnt = 0;
  if (needInsertGuids.isEmpty()) {
    return FD_OK;
  }

  auto db = GetDb();
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

  const auto& guid2Pnts = MountHelper::Guids2MntPntSet(false);
  for (const auto& vol : volumeInfos) {
    if (!needInsertGuids.contains(vol.guid)) {
      continue;
    }
    const QSet<QString>& pntsSet = guid2Pnts.value(vol.guid, {});
    const QStringList pntsLst{pntsSet.cbegin(), pntsSet.cend()};
    query.bindValue(ROOT_PATH, vol.rootPath);  // no known conversion from QString to QVariant. Causes: header QVariant not include
    query.bindValue(VOLUME_LABEL, vol.volumeLabel);
    query.bindValue(TOTAL_BYTES, vol.totalBytes);
    query.bindValue(AVAIL_BYTES, vol.availBytes);
    query.bindValue(GUID, vol.guid);
    query.bindValue(MOUNT_POINT, pntsLst.join('\n'));
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
  insertCnt = needInsertGuids.size();
  qDebug("%d record(s) to be insert...", insertCnt);
  return FD_OK;
}

FD_ERROR_CODE DevicesAndDriverDb::Delete(const QString& tableName, const QSet<QString>& needDeleteGuids, int& deleteCnt) {
  auto db = GetDb();
  deleteCnt = 0;
  if (needDeleteGuids.isEmpty()) {
    return FD_OK;
  }
  QString qryCmd = QString(R"(DELETE FROM %1 WHERE `%2` IN (")").arg(tableName, ENUM_2_STR(GUID));
  const QStringList guids{needDeleteGuids.cbegin(), needDeleteGuids.cend()};
  qryCmd += guids.join(R"(",")");
  qryCmd += R"(");)";
  QSqlQuery query{db};
  if (!query.exec(qryCmd)) {
    db.rollback();
    qWarning("delete[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return FD_INVALID;
  }
  query.finish();
  deleteCnt = needDeleteGuids.size();
  qDebug("%d record(s) to be deleted...", deleteCnt);
  return FD_OK;
}
FD_ERROR_CODE DevicesAndDriverDb::Update(const QString& tableName, const QSet<QString>& needUpdateGuids, const QList<VolumeInfo>& volumeInfos, int& updateCnt) {
  auto db = GetDb();
  updateCnt = 0;
  if (needUpdateGuids.isEmpty()) {
    return FD_OK;
  }
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
  const auto& guid2Pnts = MountHelper::Guids2MntPntSet(false);
  for (const auto& vol : volumeInfos) {
    if (!needUpdateGuids.contains(vol.guid)) {
      continue;
    }
    const QSet<QString>& pntsSet = guid2Pnts.value(vol.guid, {});
    const QStringList pntsLst{pntsSet.cbegin(), pntsSet.cend()};
    query.bindValue(UPDATE_SIZE_FIELD_ROOT_PATH, vol.rootPath);
    query.bindValue(UPDATE_SIZE_FIELD_VOLUME_LABEL, vol.volumeLabel);
    query.bindValue(UPDATE_SIZE_FIELD_TOTAL_BYTES, vol.totalBytes);
    query.bindValue(UPDATE_SIZE_FIELD_AVAIL_BYTES, vol.availBytes);
    query.bindValue(UPDATE_SIZE_FIELD_GUID, vol.guid);
    query.bindValue(UPDATE_SIZE_FIELD_MOUNT_POINT, pntsLst.join('\n'));
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
  updateCnt = needUpdateGuids.size();
  qDebug("%d record(s) to be update...", updateCnt);
  return FD_OK;
}

FD_ERROR_CODE DevicesAndDriverDb::AdtDeviceAndDriver(const QString& tableName, VolumeUpdateResult* pRst, FUNC_VOLUME_INFO_GETTER pGetter) {
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

  if (!db.tables().contains(tableName)) {
    return FD_TABLE_INEXIST;
  }

  QSet<QString> existedGuids, newGuids;
  if (!QueryPK(tableName, ENUM_2_STR(GUID), existedGuids)) {
    qWarning("Qry guid(s) at table[%s] failed", qPrintable(tableName));
    return FD_QRY_PK_FAILED;
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

  MountHelper::Guids2MntPntSet(true);
  // before insert check if at least 1 guid
  int insertCnt{0};
  auto ret = Insert(tableName, needInsertGuids, volumeInfos, insertCnt);
  if (ret != FD_OK) {
    qWarning("Insert failed errorCode:%d", ret);
    return ret;
  }

  // before delete check if at least 1 guid
  int deleteCnt{0};
  ret = Delete(tableName, needDeleteGuids, deleteCnt);
  if (ret != FD_OK) {
    qWarning("Delete failed errorCode:%d", ret);
    return ret;
  }

  // before update check if at least 1 guid
  int updateCnt{0};
  ret = Update(tableName, needUpdateGuids, volumeInfos, updateCnt);
  if (ret != FD_OK) {
    qWarning("Update failed errorCode:%d", ret);
    return ret;
  }

  if (pRst != nullptr) {
    pRst->insertCnt = insertCnt;
    pRst->deleteCnt = deleteCnt;
    pRst->updateCnt = updateCnt;
  }

  qDebug("insert:%d, delete:%d, update:%d record(s) succeed", insertCnt, deleteCnt, updateCnt);
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
